#include <time.h>

struct tm *localtime(const time_t *t)
{
    static struct tm tm;
    return localtime_r(t, &tm);
}

struct tm *localtime_r(const time_t *restrict t, struct tm *restrict tm)
{
    // TODO
}
