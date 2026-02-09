#include "time.h"

extern char *tzname[2];
extern void tzset();
extern long timezone;
extern int daylight;

struct tm *localtime(const time_t *t)
{
    static struct tm tm;
    return localtime_r(t, &tm);
}

// convert UTC time stamp `t` to `tm` expressed relative to time zone
struct tm *localtime_r(const time_t *restrict t, struct tm *restrict tm)
{
    time_t rel = *t - timezone;
    if (!gmtime_r(&rel, tm)) return 0;
    tm->tm_isdst = 0;
    tm->tm_gmtoff = -timezone;
    tm->tm_zone = tzname[0];
    return tm;
}
