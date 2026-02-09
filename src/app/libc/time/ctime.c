#include <time.h>

char *ctime(const time_t *t)
{
    static char buf[26];
    return asctime(localtime(t));
}

char *ctime_r(const time_t *restrict t, char *restrict buf)
{
    return asctime_r(localtime(t), buf);
}
