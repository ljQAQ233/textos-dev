#include <textos/signal.h>
#include <textos/syscall.h>
#include <textos/task.h>

static void kill_timeout(void *arg)
{
    task_t *tsk = (task_t *)arg;
    fkill(tsk->pid, SIGALRM);
}

__SYSCALL_DEFINE1(unsigned, alarm, unsigned, seconds)
{
    task_t *tsk = task_current();
    u64 rem = ktimer_remain_ms(&tsk->alarm) / 1000ull;
    assert(rem <= (unsigned)-1); // task::alarm only set / fire here
    if (seconds == 0)
        ktimer_kill(&tsk->alarm);
    else
        ktimer(&tsk->btmr, kill_timeout, tsk, seconds * 1000ull);
    return (unsigned)rem;
}
