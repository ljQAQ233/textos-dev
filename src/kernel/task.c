#include <cpu.h>
#include <textos/task.h>
#include <textos/mycpu.h>
#include <textos/errno.h>
#include <textos/panic.h>
#include <textos/assert.h>
#include <textos/mm/vmm.h>
#include <textos/mm/pvpage.h>
#include <textos/klib/string.h>

MYCPU_DEFINE(addr_t, current_istk);
MYCPU_DEFINE(addr_t, current_oldsp);

#define TASK_PAGE (1)
#define TASK_SIZ  (PAGE_SIZ * TASK_PAGE)

/* Each task has this def tick -> 1 */
#define TASK_TICKS 1

task_t *table[TASK_MAX];

#define TASK_FREE NULL

static int _getfree ()
{
    for (int i = 0; i < TASK_MAX ;i++)
        if (table[i] == TASK_FREE)
            return i;

    return -1;
}

/* Register tasks into table */
static task_t *_task_create(int args)
{
    int pid;
    task_t *tsk;
    if ((pid = _getfree()) < 0)
        return NULL;

    u16 map_flgs = PE_P | PE_RW;
    if (args & TC_USER)
        map_flgs |= PE_US;
    tsk = vmm_allocpages(TASK_PAGE, map_flgs);
    tsk->ruid = tsk->euid = tsk->suid = 0;
    tsk->rgid = tsk->egid = tsk->sgid = 0;
    tsk->supgids = NULL;
    tsk->umask = 0022;
    tsk->pid = pid;
    tsk->sid = 0;
    tsk->ppid = 0;
    tsk->pgid = 0;
    tsk->stat = TASK_INI;
    ktimer_init(&tsk->btmr);
    return table[pid] = tsk;
}

#include <gdt.h>
#include <intr.h>

intr_frame_t *build_iframe (task_t *tsk, void *stack,  int args)
{
    intr_frame_t *iframe = stack - sizeof(intr_frame_t);

    // very good code, makes my test passed
    iframe->rax = 0xaaaa;
    iframe->rbx = 0xbbbb;
    iframe->rcx = 0xcccc;
    iframe->rdx = 0xdddd;
    iframe->rdi = 0x1d1d;
    iframe->rsi = 0x1e1e;
    iframe->r8  = 0x8888;
    iframe->r9  = 0x9999;
    iframe->r10 = 0x1010;
    iframe->r11 = 0x1111;
    iframe->r12 = 0x1212;
    iframe->r13 = 0x1313;
    iframe->r14 = 0x1414;
    iframe->r15 = 0x1515;
    
    iframe->rbp = (u64)stack;
    iframe->rsp = (u64)stack;

    iframe->rflags = (1 << 9);

    if (args & (TC_KERN | TC_TSK1))
        iframe->cs = KERN_CODE_SEG << 3;
    else
        iframe->cs = (USER_CODE_SEG << 3) | 3;

    if (args & TC_NINT)
        iframe->rflags &= ~(1 << 9);

    return tsk->iframe = iframe;
}

task_frame_t *build_tframe (task_t *tsk, void *stack, int args)
{
    task_frame_t *frame = stack - sizeof(intr_frame_t) - sizeof(task_frame_t);

    frame->rip = (u64)intr_exit;

    return tsk->frame = frame;
}

void task_reset_allsigs(task_t *tsk)
{
    tsk->sigcurr = 0;
    tsk->sigpend = 0;
    tsk->sigmask = 0;
    memset(tsk->sigacts, 0, sizeof(tsk->sigacts));
}

task_t *task_create(void *main, int args)
{
    task_t *tsk = _task_create(args);

    void *stack, *istack;
    if (args & (TC_TSK1 | TC_USER))
    {
        vmm_phyauto(__user_stack_bot, __user_stack_pages, PE_P | PE_RW | PE_US);
        stack = (void *)__user_stack_top;
        istack = vmm_allocpages(istk_pages, PE_P | PE_RW) + istk_pages * PAGE_SIZ;
    } else {
        stack = vmm_allocpages(4, PE_P | PE_RW) + 4 * PAGE_SIZ; // TODO: replace it
        istack = NULL;
    }

    // keep 16-byte alignment of sp
    stack -= sizeof(long);

    build_iframe(tsk, stack, args)->rip = (u64)main;
    build_tframe(tsk, stack, args);

    tsk->init.main = main;
    tsk->init.rbp = (void *)tsk->iframe->rbp;
    tsk->init.args = args;

    tsk->stat  = TASK_PRE;
    
    tsk->tick = TASK_TICKS;
    tsk->curr = TASK_TICKS;
    
    tsk->pwd = NULL; // root

    tsk->pgt = get_kppgt();
    tsk->mmap = __user_mmap_va;
    tsk->vsp = vmm_new_space(0);
    tsk->istk = (addr_t)istack;

    tsk->utime = 0;
    tsk->stime = 0;
    tsk->_ustart = 0;
    tsk->_sstart = arch_us_ran();

    for (int i = 0 ; i < MAX_FILE ; i++)
        tsk->files[i] = NULL;
    task_reset_allsigs(tsk);
    return tsk;
}

#include <textos/mm.h>
#include <textos/mm/pvpage.h>

static int fork_stack(task_t *prt, task_t *chd)
{
    void *istk = vmm_allocpages(istk_pages, PE_P | PE_RW) + istk_pages * PAGE_SIZ;
    build_tframe(chd, istk, 0);
    build_iframe(chd, istk, 0);

    // 子进程在调度时才正式启动, task_frame 手动构建
    memcpy(chd->iframe, prt->sframe, sizeof(intr_frame_t));
    chd->istk = (addr_t)istk;
    return 0;
}

static int fork_pgt(task_t *prt, task_t *chd)
{
    chd->pgt = copy_pgtd(prt->pgt);
    chd->mmap = prt->mmap;
    chd->vsp = vmm_new_space(prt->vsp);
    return 0;
}

static int fork_fd(task_t *prt, task_t *chd)
{
    for (int i = 0 ; i < MAX_FILE ; i++)
    {
        chd->files[i] = prt->files[i];
        if (chd->files[i])
            chd->files[i]->refer++;
    }
    return 0;
}

static int fork_sig(task_t *prt, task_t *chd)
{
    chd->sigcurr = prt->sigcurr;
    chd->sigpend = prt->sigpend;
    chd->sigmask = prt->sigmask;
    for (int i = 0 ; i < _NSIG ; i++)
        chd->sigacts[i] = prt->sigacts[i];
    return 0;
}

static int fork_id(task_t *prt, task_t *chd)
{
    chd->ruid = prt->ruid;
    chd->euid = prt->euid;
    chd->suid = prt->suid;
    chd->rgid = prt->rgid;
    chd->egid = prt->egid;
    chd->sgid = prt->sgid;
    chd->supgids = NULL;

    if (prt->supgids)
    {
        int ss = 0;
        while (prt->supgids[ss] != -1)
            ss++;
        chd->supgids = malloc((ss + 1) * sizeof(gid_t));
        chd->supgids[ss] = -1;
        for (int i = 0 ; i < ss ; i++)
            chd->supgids[i] = prt->supgids[i];
    }

    chd->sid = prt->sid;
    chd->pgid = prt->pgid;
    return 0;
}

#include <irq.h>
#include <textos/syscall.h>

int task_fork()
{
    task_t *prt = task_current();
    task_t *chd = _task_create(prt->init.args);

    // page table
    fork_pgt(prt, chd);

    // context
    fork_stack(prt, chd);
    if (prt->sframe->vector == INT_MSYSCALL)
        chd->frame->rip = (u64)msyscall_exit;
    else
        chd->frame->rip = (u64)intr_exit;
    chd->iframe->rax = 0; // 子进程返回 0

    // files
    fork_fd(prt, chd);

    // signals
    fork_sig(prt, chd);

    // ids
    fork_id(prt, chd);

    chd->tick = prt->tick;
    chd->curr = prt->curr;
    chd->ppid = prt->pid;
    chd->pwd = prt->pwd;
    chd->stat = TASK_PRE;

    chd->init = prt->init;
    chd->did_exec = false;
    chd->utime = 0;
    chd->stime = 0;
    chd->_ustart = 0;
    chd->_sstart = arch_us_ran();
    DEBUGK(K_TRACE, "fork %d -> child=%d\n", prt->pid, chd->pid);

    return chd->pid; // 父进程返回子进程号
}

void task_exit(int val)
{
    task_t *tsk = task_current();
    tsk->stat = TASK_DIE; // TODO : ZOMBIE
    tsk->retval = val;
    task_t *prt = table[tsk->ppid];
    if (prt->stat == TASK_BLK && (prt->waitpid == tsk->pid || prt->waitpid == -1))
    {
        task_unblock(prt, 0);
        prt->waitpid = tsk->pid;
    }
    DEBUGK(K_TRACE, "exit %d\n", tsk->pid);

    if (tsk->pid == 1)
        PANIC("init exited with %d!!!\n", val);

    task_schedule();
}

int task_wait(int pid, int *stat, int opt, struct rusage *ru)
{
    task_t *tsk = task_current();
    if (pid > 0)
    {
        if (!table[pid] || table[pid]->ppid != tsk->pid)
            return -ECHILD;
    }
    else if (pid == -1)
    {
        int has = 0;
        for (int i = 0 ; i < TASK_MAX ; i++)
        {
            if (table[i] && table[i]->stat != TASK_DIE &&
                table[i]->ppid == tsk->pid)
            {
                has = 1;
                break;
            }
        }
        if (!has)
            return -ECHILD;
    }
    else
    {
        PANIC("unsupported pid - %d\n", pid);
    }
        
    tsk->waitpid = pid;
    task_block(NULL, NULL, TASK_BLK, 0);

    int termd = tsk->waitpid;
    tsk->waitpid = 0;

    task_t *chd = table[termd];
    if (ru) task_look_rusage(chd, ru);
    if (stat) *stat = chd->retval;
    table[termd] = NULL;

    return termd;
}

static void _task_kern ()
{
    task_create (NULL, TC_KERN)->stat = TASK_RUN;
}

static int _curr;

static inline task_t *_getnext ()
{
    task_t *tsk = NULL;

    for (int i = _curr, j = 0; i < TASK_MAX && j < _curr + TASK_MAX
            ;i = (i + 1) % TASK_MAX, j++)
    {
        if (table[i] == TASK_FREE)
            continue;
        if (table[i]->stat == TASK_PRE) {
            _curr = i;
            return table[i];
        }
    }

    return NULL;
}

task_t *task_current ()
{
    return table[_curr];
}

task_t *task_get(int pid)
{
    if (pid < 0 || pid >= TASK_MAX)
        return NULL;
    return table[pid];
}

extern void fpu_disable();
extern void __tss_set(u64 rsp);
extern void __task_switch(task_frame_t *next, task_frame_t **curr);

void task_schedule()
{
    task_t *curr, *next;
    
    curr = task_current(); /* Get curr first becase _getnext() will change `_curr` */
    if (!curr)
        return;
    
    if (curr->stat != TASK_RUN)   // If it is not running now, then sched
        goto Sched;               // Because a running task may have some time ticks
    if (curr->curr-- == 0)        // Update time ticks, if it is zero after this time, set it to origin ticks
        curr->curr = curr->tick;  // Recovery -> gain ticks again
    else
        return;                   // It hasn't ran out of all ticks yet, make it run again

Sched:
    if (!(next = _getnext()))
        return;

    if (curr->stat == TASK_RUN)   // The current task may be a sleeping task
        curr->stat = TASK_PRE;    // Only the running task which will be switched out
    next->stat = TASK_RUN;

    fpu_disable();
    __tss_set(next->istk);
    mycpu_var(current_istk) = next->istk;
    write_cr3(next->pgt);
    __task_switch (next->frame, &curr->frame);
}

void task_yield()
{
    task_schedule();
}

static list_t list_block;
static list_t list_sleep;

static void wake_timeout(void *tsk)
{
    task_unblock(tsk, -ETIME);
}

int task_block(task_t *tsk, list_t *blist, int stat, u64 timeout)
{
    if (tsk == NULL)
        tsk = task_current();

    tsk->stat = TASK_BLK;
    tsk->retval = 0;
    list_insert(blist ? blist : &list_block, &tsk->blist);
    
    if (timeout > 0)
        ktimer(&tsk->btmr, wake_timeout, tsk, timeout);

    if (tsk == task_current())
        task_schedule();
    return tsk->retval;
}

void task_unblock(task_t *tsk, int reason)
{
    ASSERTK(tsk != NULL);
    tsk->retval = reason;
    tsk->stat = TASK_PRE;
    task_stime_discard();
}

int task_sleep(u64 ms, u64 *rms)
{
    task_t *curr = task_current();
    task_block(curr, &list_sleep, TASK_SLP, ms);
    task_stime_discard();
    if (rms)
        *rms = ktimer_remain(&curr->btmr);
    return curr->retval;
}

void task_stime_enter()
{
    task_t *tsk = task_current();
    useconds_t us = arch_us_ran(); 
    tsk->_sstart = us;
    tsk->utime += us - tsk->_ustart;
}

void task_stime_exit()
{
    task_t *tsk = task_current();
    useconds_t us = arch_us_ran(); 
    tsk->_ustart = us;
    tsk->stime += us - tsk->_sstart;
}

void task_stime_discard()
{
    task_t *tsk = task_current();
    tsk->_sstart = arch_us_ran();
}

static void us_to_timeval(u64 us, struct timeval *tp)
{
    useconds_t scale = 1000 * 1000;
    tp->tv_sec = us / scale;
    tp->tv_usec = us % scale;
}

void task_look_rusage(task_t *tsk, struct rusage *ru)
{
    memset(ru, 0, sizeof(struct rusage));
    us_to_timeval(tsk->utime, &ru->ru_utime);
    us_to_timeval(tsk->stime, &ru->ru_stime);
}

#include <textos/syscall.h>

/*
 * syscalls
 */
__SYSCALL_DEFINE0(int, fork)
{
    return task_fork();
}

__SYSCALL_DEFINE1(RETVAL(void), exit, int, stat)
{
    task_exit(make_stat_exit(stat));
    __builtin_unreachable();
}

// uninterrupt
__SYSCALL_DEFINE4(int, wait4, int, pid, int *, stat, int, opt, void *, rusage)
{
    return task_wait(pid, stat, opt, rusage);
}

__SYSCALL_DEFINE2(int, getrusage, int, who, struct rusage *, ru)
{
    if (ru == NULL) return -EINVAL;
    switch (who) {
    case RUSAGE_SELF: {
        task_t *tsk = task_current();
        task_look_rusage(tsk, ru);
    } break;
    case RUSAGE_CHILDREN:
    case RUSAGE_THREAD:
        return -ENOSYS;
    default:
        return -EINVAL;
    }
    return 0;
}

__SYSCALL_DEFINE0(uid_t, getuid)
{
    return task_current()->ruid;
}

__SYSCALL_DEFINE0(gid_t, getgid)
{
    return task_current()->rgid;
}

__SYSCALL_DEFINE0(uid_t, geteuid)
{
    return task_current()->euid;
}

__SYSCALL_DEFINE0(gid_t, getegid)
{
    return task_current()->egid;
}

__SYSCALL_DEFINE1(int, setuid, uid_t, uid)
{
    task_t *tsk = task_current();
    if (tsk->euid == 0)
    {
        tsk->ruid = tsk->euid = tsk->suid = uid;
    }
    else
    {
        if (uid == tsk->ruid || uid == tsk->suid)
            tsk->euid = uid;
        else
            return -EPERM;
    }
    return 0;
}

__SYSCALL_DEFINE1(int, setgid, gid_t, gid)
{
    task_t *tsk = task_current();
    if (tsk->egid == 0)
    {
        tsk->rgid = tsk->egid = tsk->sgid = gid;
    }
    else
    {
        if (gid == tsk->rgid || gid == tsk->sgid)
            tsk->egid = gid;
        else
            return -EPERM;
    }
    return 0;
}

__SYSCALL_DEFINE2(int, setreuid, uid_t, ruid, uid_t, euid)
{
    task_t *tsk = task_current();
    /*
     * A process with appropriate privileges can set either id to any value.
     * Other processes can only set the effective user id if the euid argument
     * is equal to either the real, effective, or saved user id of the process.
     * POSIX doesn't specify whether the later can change ruid, here we disallow it.
     */
    if (tsk->euid != 0)
    {
        if (ruid != -1)
            return -EPERM;
        if (euid != -1 && euid != tsk->ruid && euid != tsk->euid && euid != tsk->suid)
            return -EPERM;
    }
    if (ruid != -1) tsk->ruid = ruid;
    if (euid != -1) tsk->euid = euid;
    return 0;
}

__SYSCALL_DEFINE2(int, setregid, gid_t, rgid, gid_t, egid)
{
    task_t *tsk = task_current();
    /*
     * Unlike ruid, a non-privileged proc can not set its gid at will.
     * However, there are two exceptions:
     *   - rgid can be set to sgid
     *   - egid can be set to sgid / rgid
     * Keep this syscall atomic!!!
     */
    if (tsk->egid != 0)
    {
        if (rgid != -1 && rgid != tsk->sgid)
                return -EPERM;
        if (egid != -1 && egid != tsk->sgid && egid != tsk->rgid)
                return -EPERM;
    }
    if (rgid != -1) tsk->rgid = rgid;
    if (egid != -1) tsk->egid = egid;
    return 0;
}

__SYSCALL_DEFINE2(int, getgroups, int, size, gid_t *, list)
{
    task_t *tsk = task_current();
    int ss = 0;
    if (tsk->supgids)
        while (tsk->supgids[ss] != -1)
            ss++;
    if (!list && !size)
        return ss;
    else if (!list)
        return -EFAULT;
    else if (size < ss)
        return -EINVAL;
    for (int i = 0 ; i < ss ; i++)
        list[i] = tsk->supgids[i];
    return ss;
}

__SYSCALL_DEFINE2(int, setgroups, int, size, gid_t *, list)
{
    task_t *tsk = task_current();
    if (!size)
        return getgroups(0, NULL);
    if (size < 0)
        return -EINVAL;
    if (!list)
        return -EFAULT;
    if (tsk->euid != 0)
        return -EPERM;
    if (tsk->supgids)
    {
        free(tsk->supgids);
        tsk->supgids = NULL;
    }
    for (int i = 0 ; i < size ; i++)
        if (list[i] == -1)
            return -EINVAL;
    tsk->supgids = malloc(sizeof(gid_t) * (size + 1));
    tsk->supgids[size] = -1;
    for (int i = 0 ; i < size ; i++)
        tsk->supgids[i] = list[i];
    return 0;
}

__SYSCALL_DEFINE1(pid_t, getsid, pid_t, pid)
{
    task_t *tsk = task_current();
    if (pid == 0)
        return tsk->sid;
    task_t *ptsk = task_get(pid);
    if (!ptsk)
        return -ESRCH;
    if (ptsk->sid != tsk->sid)
        return -EPERM;
    return ptsk->sid;
}

__SYSCALL_DEFINE0(pid_t, setsid)
{
    task_t *tsk = task_current();
    if (tsk->pgid == tsk->pid)
        return -EPERM;
    return tsk->sid = tsk->pgid = tsk->pid;
}

__SYSCALL_DEFINE1(pid_t, getpgid, pid_t, pid)
{
    task_t *tsk = task_current();
    task_t *ptsk;
    if (pid < 0)
        return -EINVAL;

    if (pid == 0)
        pid = (ptsk = tsk)->pid;
    else
        ptsk = task_get(pid);
    
    if (!ptsk)
        return -ESRCH;
    if (ptsk->sid != tsk->sid)
        return -EPERM;
    return ptsk->pgid;
}

__SYSCALL_DEFINE2(int, setpgid, pid_t, pid, pid_t, pgid)
{
    /*
     * To provide tighter security, setpgid() only allows the calling process
     * to join a process group already in use inside its session or create a
     * new process group whose process group ID was equal to its process ID.
     */
    task_t *tsk = task_current();
    task_t *ptsk;
    task_t *ltsk;

    if (pid < 0)
        return -EINVAL;

    if (pid == 0)
        pid = (ptsk = tsk)->pid;
    else
        ptsk = task_get(pid);

    /*
     * Is `ptsk` the calling process itself or one of its child process?
     * Is `ptsk` a leader of a session? -> Forbidden by POSIX.
     */
    if (!ptsk)
        return -ESRCH;
    if (ptsk->pid != tsk->pid && ptsk->ppid != tsk->pid)
        return -ESRCH;
    if (ptsk->pid == ptsk->sid)
        return -EPERM;

    if (pgid == 0)
        pgid = (ltsk = tsk)->pid;
    else
        ltsk = task_get(pgid);

    if (!ltsk)
        return -ESRCH;
    if (ptsk->sid != ltsk->sid)
        return -EPERM;
    if (ptsk->did_exec)
        return -EACCES;

    ptsk->pgid = pgid;
    return 0;
}

__SYSCALL_DEFINE0(pid_t, getpid)
{
    return task_current()->pid;
}

__SYSCALL_DEFINE0(pid_t, getppid)
{
    return task_current()->ppid;
}

__SYSCALL_DEFINE1(void *, brk, void *, ptr)
{
    addr_t ask = (addr_t)ptr;
    task_t *tsk = task_current();
    if (ptr == NULL)
        return MRET(tsk->brk);
    if (ask < __user_heap_va)
        return MRET(-ENOMEM);
    ask = (ask + PAGE_SIZE) & PAGE_MASK;
    return MRET(tsk->brk = ask);
}

__SYSCALL_DEFINE0(int, yield)
{
    task_schedule();
    return 0;
}

__SYSCALL_DEFINE2(int, nanosleep, const struct timespec *, rqtp, struct timespec *, rmtp)
{
    int ret;
    u64 ms = 0, rms;
    ms += rqtp->tv_sec * 1000;
    ms += rqtp->tv_nsec / 1000 / 1000;
    ret = task_sleep(ms, &rms);
    rmtp->tv_sec = rms / 1000;
    rmtp->tv_nsec = (rms % 1000) * 1000 * 1000;
    return ret;
}

#include <bits/times.h>

__SYSCALL_DEFINE1(clock_t, times, struct tms *, buf)
{
    task_t *tsk = task_current();
    useconds_t cutime = 0;
    useconds_t cstime = 0;
    for (int i = 0 ; i < TASK_MAX ; i++) {
        if (task_get(i)->ppid == tsk->pid) {
            cutime += tsk->utime;
            cstime += tsk->stime;
        }
    }
    buf->tms_utime = tsk->utime / CLK_TCK;
    buf->tms_stime = tsk->stime / CLK_TCK;
    buf->tms_cutime = cutime / CLK_TCK;
    buf->tms_cstime = cstime / CLK_TCK;
    return arch_us_ran() / CLK_TCK;
}

void task_init ()
{
    for (int i = 0; i < TASK_MAX ;i++)
    {
        table[i] = TASK_FREE;
    }
    list_init(&list_block);
    list_init(&list_sleep);

    _task_kern();
    _curr = 0;
}
