#ifndef __TASK_H__
#define __TASK_H__

// #include <Cpu.h>

#include <intr.h>

typedef struct {
    u64  r15;
    u64  r14;
    u64  r13;
    u64  r12;
    u64  rbx;
    u64  rbp;
    u64  rip;
} task_frame_t;

typedef struct _Task {
    int pid;
    int stat;

    task_frame_t *frame;
    intr_frame_t *iframe;
} task_t;

#define TASK_DIE  0 // Dead
#define TASK_PRE  1 // Prepared
#define TASK_RUN  2 // Running

void task_schedule ();

void task_yield ();

task_t *task_current ();

task_t *task_create (void *main);

#endif
