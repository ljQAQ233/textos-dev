#ifndef _TIME_H
#define _TIME_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_size_t
#define __NEED_time_t
#define __NEED_clock_t
#define __NEED_struct_timespec
#include <bits/null.h>
#include <bits/alltypes.h>

time_t time(time_t /* _Nullable */ *__tp);

int nanosleep(const struct timespec *__rqtp, struct timespec *__rmtp);

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long tm_gmtoff;
    const char *tm_zone;
};

char *asctime(const struct tm *__tm);
char *asctime_r(const struct tm *restrict __tm, char *restrict __buf);

char *ctime(const time_t *__t);
char *ctime_r(const time_t *restrict __t, char *restrict __buf);

struct tm *gmtime(const time_t *__t);
struct tm *gmtime_r(const time_t *restrict __t, struct tm *restrict __tm);

struct tm *localtime(const time_t *__t);
struct tm *localtime_r(const time_t *restrict __t, struct tm *restrict __tm);

time_t mktime(struct tm *tm);

__END_DECLS

#endif
