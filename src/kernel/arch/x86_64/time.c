#include <cpu.h>
#include <textos/time.h>
#include <textos/dev/pit.h>

/*
 * _time -> time stamp since Epoch
 * _archtick -> arch-specified tick source
 */
time_t __startup_time;
u64 __startup_archtick;

static size_t tsc_divisor;

time_t arch_time_ran()
{
    u64 us = arch_us_ran();
    return us / 1000ull / 1000ull;
}

time_t arch_time_now()
{
    return arch_time_ran() + __startup_time;
}

suseconds_t arch_us_ran()
{
    if (!tsc_divisor)
    {
        u64 s, t;
        s = read_tsc();
        pit_sleepms(2);
        t = read_tsc();
        tsc_divisor = (t - s + 1000) / 2000;
    }

    u64 delta = read_tsc() - __startup_archtick;
    return delta / tsc_divisor;
}

suseconds_t arch_us_thissec()
{
    return arch_us_ran() % (1000ull * 1000ull);
}
