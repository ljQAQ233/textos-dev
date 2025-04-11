#include <textos/mm.h>
#include <textos/tick.h>
#include <textos/ktimer.h>
#include <textos/klib/list.h>

static list_t timers = LIST_INIT(timers);

void ktimer(
        ktimer_t *tmr,
        void (*func)(void *arg),
        void *arg, u64 ms
        )
{
    tmr->func = func;
    tmr->arg = arg;
    tmr->dead = false;
    tmr->expires = __ktick + ms;
    list_insert(&timers, &tmr->list);
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
        
        list_remove(ptr);
        tmr->func(tmr->arg);
        tmr->dead = true;
    }
}