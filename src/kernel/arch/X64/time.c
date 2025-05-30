#include <cpu.h>
#include <textos/time.h>
#include <textos/dev/pit.h>

time_t __startup_time;

static size_t tsc_divisor;

time_t arch_time_now()
{
    if (!tsc_divisor)
    {
        u64 s, t;
        s = read_tsc();
        pit_sleepms(2);
        t = read_tsc();
        tsc_divisor = (t - s + 1000) / 2000;
    }

    u64 us = read_tsc() / tsc_divisor;
    return us / 1000ull / 1000ull + __startup_time;
}

