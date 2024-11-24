#ifndef __LOCK__
#define __LOCK__

#include <textos/task.h>
#include <textos/klib/list.h>

// 或者使用 二元信号量, 也叫 mutex
typedef struct
{
    int val;
    list_t waiter;
} semaphore_t;

void sema_init(semaphore_t *s, int val);

void sema_up(semaphore_t *s);

void sema_down(semaphore_t *s);

typedef struct
{
    task_t *holder;
    unsigned hrepeat;
    semaphore_t sema;
} lock_t;

void lock_init(lock_t *lock);

void lock_acquire(lock_t *lock);

void lock_release(lock_t *lock);

#endif
