#include <textos/time.h>
#include <textos/syscall.h>

__SYSCALL_DEFINE2(int, gettimeofday, struct timeval *, tp, void *, tzp)
{
    tp->tv_sec = arch_time_now();
    tp->tv_usec = arch_us_thissec();
    return 0;
}
