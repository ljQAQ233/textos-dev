#ifndef __TASK_H__
#define __TASK_H__

// #include <Cpu.h>

#include <intr.h>

typedef struct
{
    u64  r15;
    u64  r14;
    u64  r13;
    u64  r12;
    u64  rbx;
    u64  rbp;
    u64  rip;
} task_frame_t;

#include <bits/rusage.h>
#include <textos/file.h>
#include <textos/ktimer.h>
#include <textos/signal.h>
#include <textos/mm/mman.h>
#include <textos/klib/list.h>

#define MAX_FILE 16

typedef struct task
{
    addr_t istk; // tss
    task_frame_t *frame;
    intr_frame_t *iframe;
    intr_frame_t *sframe;
    addr_t oldsp;
    
    uid_t ruid; // real uid
    uid_t euid; // effective uid
    uid_t suid; // saved set-uid
    gid_t rgid; // real gid
    gid_t egid; // effective gid
    gid_t sgid; // saved set-gid
    gid_t *supgids;
    u16 umask;
    int pid;
    int sid;
    int ppid;
    int pgid;
    int stat;
    u64 tick;  // default ticks it has
    u64 curr;  // current ticks it has

    node_t *pwd;
    file_t *files[MAX_FILE];

    struct
    {
        int args;
        void *main;
        void *rbp;
    } init;
    bool did_exec;

    addr_t pgt;
    addr_t mmap;
    addr_t brk;
    vm_space_t *vsp;
    void *fpu;

    int retval;
    int waitpid;
    u64 utime;
    u64 stime;
    u64 _ustart;
    u64 _sstart;

    ktimer_t btmr;
    list_t blist;
    int sigcurr;
    sigset_t sigpend;
    sigset_t sigmask;
    sigaction_t sigacts[_NSIG];
} task_t;

#define TASK_DIE  0 // Dead
#define TASK_PRE  1 // Prepared
#define TASK_RUN  2 // Running
#define TASK_SLP  3 // Sleep
#define TASK_BLK  4 // Blocked
#define TASK_STP  5 // Stoped
#define TASK_INI  6 // Initializing

void task_schedule();
void task_yield();

task_t *task_current();
task_t *task_get(int pid);

#define TC_KERN (0 << 0) // ring 0 (kernel)
#define TC_USER (1 << 0) // ring 3 (init / app)
#define TC_TSK1 (1 << 1) // init proc
#define TC_NINT (1 << 2) // mask interrupt :(

void task_reset_allsigs(task_t *tsk);
task_t *task_create(void *main, int args);

int task_fork();

int task_block(task_t *tsk, list_t *blist, int stat, u64 timeout);
void task_unblock(task_t *tsk, int reason);
int task_sleep(u64 ms, u64 *rms);

void task_exit(int val);
int task_wait(int pid, int *stat, int opt, struct rusage *ru);

/* mark stime's start and end, or discard the current count */
void task_stime_enter();
void task_stime_exit();
void task_stime_discard();
void task_look_rusage(task_t *tsk, struct rusage *ru);

#define TASK_MAX 16

extern task_t *table[TASK_MAX];

#endif
