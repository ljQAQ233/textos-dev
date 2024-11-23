#include <cpu.h>
#include <textos/task.h>
#include <textos/mm/vmm.h>
#include <textos/mm/pvpage.h>
#include <textos/assert.h>

#include <string.h>

#define TASK_MAX  16

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
static task_t *_task_create (int args)
{
    int pid;
    task_t *tsk;

    if ((pid = _getfree()) < 0)
        return NULL;

    u16 map_flgs = PE_P | PE_RW;
    if (args & TC_USER)
        map_flgs |= PE_US;
    tsk = vmm_allocpages (TASK_PAGE, map_flgs);
    tsk->pid = pid;

    table[pid] = tsk;

    return tsk;
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
        istack = vmm_allocpages(1, PE_P | PE_RW);
    } else {
        stack = vmm_allocpages(1, PE_P | PE_RW); // TODO: replace it
        istack = NULL;
    }

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
    tsk->istk = (addr_t)istack;

    for (int i = 0 ; i < MAX_FILE ; i++)
        tsk->files[i] = NULL;
    tsk->files[STDIN_FILENO] = &sysfile[STDIN_FILENO];
    tsk->files[STDOUT_FILENO] = &sysfile[STDOUT_FILENO];
    tsk->files[STDERR_FILENO] = &sysfile[STDERR_FILENO];

    return tsk;
}

#include <textos/mm.h>
#include <textos/mm/pvpage.h>

static int fork_stack(task_t *prt, task_t *chd)
{
    void *istk = vmm_allocpages(istk_pages, PE_P | PE_RW);
    build_tframe(chd, istk, 0); // 较正
    build_iframe(chd, istk, 0);
    memcpy(chd->frame, prt->frame, sizeof(task_frame_t));
    memcpy(chd->iframe, prt->iframe, sizeof(intr_frame_t));
    chd->istk = (addr_t)istk;
    return 0;
}

static int fork_pgt(task_t *prt, task_t *chd)
{
    chd->pgt = copy_pgtd(prt->pgt);
    return 0;
}

static int fork_fd(task_t *prt, task_t *chd)
{
    for (int i = 0 ; i < MAX_FILE ; i++)
        chd->files[i] = prt->files[i];
    return 0;
}

#include <irq.h>
#include <textos/syscall.h>

int task_fork()
{
    task_t *prt = task_current();
    task_t *chd = _task_create(prt->init.args);
    
    // prevent task switching
    // because currently we use iframe to save context for fork
    UNINTR_AREA_START();

    // page table
    fork_pgt(prt, chd);

    // context
    fork_stack(prt, chd);
    if (prt->iframe->vector == INT_MSYSCALL)
        chd->frame->rip = (u64)msyscall_exit;
    else
        chd->frame->rip = (u64)intr_exit;
    chd->iframe->rax = 0; // 子进程返回 0

    // files
    fork_fd(prt, chd);

    chd->tick = prt->tick;
    chd->curr = prt->curr;
    chd->ppid = prt->pid;
    chd->pwd = prt->pwd;
    chd->stat = TASK_PRE;

    UNINTR_AREA_END();

    return chd->pid; // 父进程返回子进程号
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

void __task_setif(void *iframe)
{
    task_current()->iframe = iframe;
}

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

#include <textos/dev.h>
#include <textos/printk.h>

#define N 5000

static int a = 0;

extern void ide_read (u32 lba, void *data, u8 cnt);

void proc_a ()
{
    u8 buf[512];
    dev_t *ide = dev_lookup_type (DEV_BLK, DEV_IDE);
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

