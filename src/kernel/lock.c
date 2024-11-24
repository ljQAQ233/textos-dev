#include <textos/lock.h>
#include <textos/task.h>
#include <textos/assert.h>

#include <irq.h>

void sema_init(semaphore_t *s, int val)
{
    s->val = val;
    list_init(&s->waiter);
}

void sema_up(semaphore_t *s)
{
    UNINTR_AREA_START();

    ASSERTK(s->val == 0);
    s->val += 1;
    ASSERTK(s->val == 1);

    if (!list_empty(&s->waiter)) {
        list_t *first = s->waiter.next;
        list_remove(first);
        task_unblock(CR(first, task_t, waiting)->pid);
    }

    UNINTR_AREA_END();
}

void sema_down(semaphore_t *s)
{
    UNINTR_AREA_START();

    if (s->val == 0) {
        list_insert(&s->waiter, &task_current()->waiting);
        task_block();
    }

    ASSERTK(s->val == 1);
    s->val -= 1;
    ASSERTK(s->val == 0);

    UNINTR_AREA_END();
}

void lock_init(lock_t *lock)
{
    lock->holder = NULL;
    lock->hrepeat = 0;
    sema_init(&lock->sema, 1);
}

void lock_acquire(lock_t *lock)
{
    task_t *tsk = task_current();
    if (lock->holder != tsk) {
        sema_down(&lock->sema);
        lock->holder = tsk;
        lock->hrepeat = 1;
    } else {
        lock->hrepeat += 1;
    }
}

void lock_release(lock_t *lock)
{
    task_t *tsk = task_current();
    ASSERTK(lock->holder == tsk);
    if (lock->hrepeat > 1) {
        lock->hrepeat -= 1;
        return ;
    }

    lock->holder = NULL;
    lock->hrepeat = 0;
    sema_up(&lock->sema);
}
