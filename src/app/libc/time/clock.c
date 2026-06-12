#include <sys/times.h>
#include <time.h>
#include <unistd.h>

clock_t clock()
{
    struct tms tms;
    if (times(&tms) == (clock_t)-1) return (clock_t)-1;
    return (tms.tms_stime + tms.tms_utime) * (CLOCKS_PER_SEC / CLK_TCK);
}
