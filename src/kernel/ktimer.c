#include <textos/mm.h>
#include <textos/tick.h>
#include <textos/ktimer.h>
#include <textos/klib/list.h>

static list_t timers = LIST_INIT(timers);

/*
 * 启动一个已经初始化好了的定时器.
 * 
 * 为了快速了解定时器是否有效, 使用 `dead` 来标记一个 timer,
 * 因此 在启动定时器之前 `ktimer_init` 是很有必要的!!!
 */
void ktimer(
        ktimer_t *tmr,
        void (*func)(void *arg),
        void *arg, u64 ms
        )
{
    tmr->func = func;
    tmr->arg = arg;
    tmr->expires = __ktick + ms;
    if (!tmr->dead)
        return ;
    tmr->dead = false;
    list_insert(&timers, &tmr->list);
}

void ktimer_init(ktimer_t *tmr)
{
    tmr->dead = true;
}

void ktimer_kill(ktimer_t *tmr)
{
    if (tmr->dead)
        return ;
    tmr->dead = true;
    list_remove(&tmr->list);
}

void ktimer_fire(ktimer_t *tmr)
{
    if (tmr->dead)
        return ;
    ktimer_kill(tmr);
    tmr->func(tmr->arg);
}

void ktimer_check()
{
    list_t *ptr = timers.next;
    list_t *nxt = ptr->next;
    for ( ; ptr != &timers ; ptr = nxt, nxt = nxt->next)
    {
        ktimer_t *tmr = CR(ptr, ktimer_t, list);
        if (tmr->expires > __ktick)
            continue;
        ktimer_fire(tmr);
    }
}

u64 ktimer_remain(ktimer_t *tmr)
{
    u64 curtick = __ktick;
    if (tmr->expires > curtick)
        return tmr->expires - curtick;
    return 0;
}
