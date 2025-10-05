#include <cpu.h>
#include <textos/task.h>
#include <textos/errno.h>
#include <textos/panic.h>
#include <textos/assert.h>
#include <textos/mm/vmm.h>
#include <textos/mm/pvpage.h>
#include <textos/klib/string.h>

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
    tsk->pid = pid;
    tsk->sid = 0;
    tsk->ppid = 0;
    tsk->pgid = 0;
    tsk->stat = TASK_INI;
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

task_t *task_create (void *main, int args)
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
    tsk->vsp = mm_new_space();
    tsk->istk = (addr_t)istack;

    for (int i = 0 ; i < MAX_FILE ; i++)
        tsk->files[i] = NULL;

    tsk->sigcurr = 0;
    tsk->sigpend = 0;
    tsk->sigmask = 0;
    memset(tsk->sigacts, 0, sizeof(tsk->sigacts));

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

    return chd->pid; // 父进程返回子进程号
}

void task_exit(int pid, int val)
{
    task_t *tsk = task_get(pid);
    if (!tsk)
        return ;

    tsk->stat = TASK_DIE;
    tsk->retval = val;

    task_t *prt = table[tsk->ppid];
    if (prt->stat == TASK_BLK && (prt->waitpid == tsk->pid || prt->waitpid == -1))
    {
        task_unblock(prt->pid);
        prt->waitpid = tsk->pid;
    }
    DEBUGK(K_TASK, "exit %d\n", tsk->pid);

    task_schedule();
}

int task_wait(int pid, int *stat, int opt, void *rusage)
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
    task_block();

    int termd = tsk->waitpid;
    tsk->waitpid = 0;

    task_t *chd = table[termd];
    if (stat)
        *stat = chd->retval;
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

extern void __task_switch (task_frame_t *next, task_frame_t **curr);

static void update_sleep ();

void task_schedule ()
{
    task_t *curr, *next;
    
    curr = task_current(); /* Get curr first becase _getnext() will change `_curr` */
    if (!curr)
        return;

    update_sleep();
    
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
    tss_set(next->istk);
    write_msr(MSR_GS_BASE, (addr_t)next);
    write_cr3(next->pgt);
    __task_switch (next->frame, &curr->frame);
}

void task_yield ()
{
    task_schedule();
}

struct list _sleeping;

static void sleep_insert (task_t *task)
{
    list_insert (&_sleeping, &task->sleeping);
}

static void update_sleep ()
{
    if (list_empty (&_sleeping)) // 没有任务在睡觉呢...
        return;

    list_t *ptr;
    LIST_FOREACH(ptr, &_sleeping)
    {
        task_t *task = CR(ptr, task_t, sleeping);
        if (task->pid == _curr)
            continue;
        if (--task->sleep == 0) {   // 坑死老子啦！2的64次方的大整数是什么 o.o
            task->stat = TASK_PRE;
            list_remove (ptr);
        }
    }
}

void task_sleep (u64 ticks)
{
    ASSERTK (ticks != 0); // 专门防止 24h 工作制

    task_t *curr = task_current();
    ASSERTK (curr != NULL);

    curr->stat = TASK_SLP;
    curr->sleep = ticks;

    sleep_insert(curr);
    task_schedule();
}

void task_block ()
{
    task_t *curr = task_current();
    curr->stat = TASK_BLK;
    task_schedule();
}

void task_unblock (int pid)
{
    task_t *tsk = table[pid];
    ASSERTK (tsk != NULL);
    tsk->stat = TASK_PRE;
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
    task_exit(task_current()->pid, stat);

    __builtin_unreachable();
}

// uninterrupt
__SYSCALL_DEFINE4(int, wait4, int, pid, int *, stat, int, opt, void *, rusage)
{
    return task_wait(pid, stat, opt, rusage);
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

__SYSCALL_DEFINE0(int, yield)
{
    task_schedule();
    return 0;
}

#include <textos/dev.h>
#include <textos/printk.h>

#define N 5000

static int a = 0;

void proc_a ()
{
    u8 buf[512];
    devst_t *ide = dev_lookup_type (DEV_IDE, 0);
    ide->bread (ide, 0, buf, 1);
    for (int i = 0 ; i < 512 ; i++)
        printk ("%02x", buf[i]);
    printk ("\n");

    while (true) { }
}

void proc_b ()
{
    for (int i = 0 ; i < N ; i++)
        printk ("PROC[#%d] a = %d\n", task_current()->pid, a++);
    while (true) { }
}

void task_init ()
{
    for (int i = 0; i < TASK_MAX ;i++)
    {
        table[i] = TASK_FREE;
    }

    list_init (&_sleeping);

    _task_kern();

    _curr = 0;

    // task_create (proc_a);
    // task_create (proc_b);
}
