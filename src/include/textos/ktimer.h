#ifndef __KTIMER_H__
#define __KTIMER_H__

/*
 * kernel timers - driven by apic timer
 */

#include <textos/klib/list.h>

typedef struct
{
    void (*func)(void *arg);
    void *arg;
    u64 expires;
    bool dead;
    list_t list;
} ktimer_t;

void ktimer(
        ktimer_t *tmr,
        void (*func)(void *arg),
        void *arg, u64 ms
        );

void ktimer_check();

#endif
