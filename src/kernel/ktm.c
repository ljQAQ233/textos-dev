/*
 * ktm - measure time spent
 * before, we use apic timer and kernel __ktick as a tick source,
 *  but __ktick doesn't increase if interrupt state is off. so,
 * HPET makes it better to hold a period easy and human-friendly,
 *  but hpet may have a non-64-bit register, which leads to overflow.
 * TSC (Time Stamp Counter) is a good choice, but its frequency
 * depends on the cpu which starts TSC as long as it starts up.
 *  but overclocking may break it.
 * In this view, TSC may be the best choice if hpet may overflow,
 * so, measure its frequency first
 */

#include <cpu.h>
#include <textos/printk.h>
#include <textos/assert.h>
#include <textos/ktm.h>
#include <textos/dev/pit.h>
#include <textos/dev/hpet.h>

/*
 * tick source
 */
static size_t (*get_us)();

static size_t tsc_divisor;

static size_t tsc_get_us()
{
    return read_tsc() / tsc_divisor;
}

void ktm_init()
{
    if (hpet_is_long()) {
        get_us = (void *)hpet_get_us;
        DEBUGK(K_INFO, "ktm : hpet used as tick source\n");
    } else {
        u64 s, t;
        s = read_tsc();
        pit_sleepms(2);
        t = read_tsc();
        tsc_divisor = (t - s + 1000) / 2000;
        get_us = (void *)tsc_get_us;
        DEBUGK(K_INFO, "ktm : tsc used as tick source\n");
    }
}

void ktm_start(ktm_t *k, char *label)
{
    ASSERTK(k != NULL);
    k->s = get_us();
    k->label = label;
}

void ktm_stop(ktm_t *k)
{
    k->t = get_us();
}

void ktm_dump(ktm_t *k)
{
    size_t us = (k->t - k->s);
    size_t ms = (k->t - k->s) / 1000ull;
    DEBUGK(K_INFO,
      "ktm : [%s] spend %ums %uus (%llu ~ %llu)\n",
      k->label, ms, us, k->s, k->t
      );
}
