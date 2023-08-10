#include <cpu.h>
#include <textos/task.h>
#include <textos/mm/vmm.h>

#include <string.h>

#define TASK_MAX  16

#define TASK_PAGE (1)
#define TASK_SIZ  (PAGE_SIZ * TASK_PAGE)

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
static task_t *_task_create ()
{
    int pid;
    task_t *tsk;

    if ((pid = _getfree()) < 0)
        return NULL;

    tsk = vmm_allocpages (TASK_PAGE, PE_P | PE_RW);
    tsk->pid = pid;

    table[pid] = tsk;

    return tsk;
}

#include <gdt.h>
#include <intr.h>

task_t *task_create (void *main)
{
    task_t *tsk = _task_create();

    void *stack = (void *)tsk + TASK_SIZ;
    intr_frame_t *iframe = stack - sizeof(intr_frame_t);
    
    iframe->rip = (u64)main;
    iframe->rflags = (1 << 9);
    iframe->cs = KERN_CODE_SEG << 3;
    iframe->rbp = (u64)stack;
    iframe->rsp = (u64)stack;

    task_frame_t *frame = stack - sizeof(intr_frame_t) - sizeof(task_frame_t);
    frame->rip = (u64)intr_exit;

    tsk->frame = frame;
    tsk->stat  = TASK_PRE;

    return tsk;
}

static void _task_kern ()
{
    task_create (NULL)->stat = TASK_RUN;
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

extern void __task_switch (task_frame_t *next, task_frame_t **curr);

void task_schedule ()
{
    task_t *curr = task_current(); /* Get curr first becase _getnext() will change `_curr` */
    if (!curr)
        return;

    task_t *next = _getnext();
    if (!next)
        return;

    curr->stat = TASK_PRE;
    next->stat = TASK_RUN;

    __task_switch (next->frame, &curr->frame);
}

void task_yield ()
{
    task_schedule();
}

#include <textos/printk.h>

#define N 5000

static int a = 0;

void proc_a ()
{
    for (int i = 0 ; i < N ; i++)
        printk ("PROC[#%d] a = %d\n", task_current()->pid, a++);
    while (true) {}
}

void proc_b ()
{
    for (int i = 0 ; i < N ; i++)
        printk ("PROC[#%d] a = %d\n", task_current()->pid, a++);
    while (true) {}
}

void task_init ()
{
    for (int i = 0; i < TASK_MAX ;i++)
    {
        table[i] = TASK_FREE;
    }

    _task_kern();

    _curr = 0;

    task_create (proc_a);
    task_create (proc_b);
}

