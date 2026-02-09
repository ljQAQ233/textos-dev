#include <time.h>
#include <stdio.h>

#define BUFMIN 26

char *asctime(const struct tm *tm)
{
    static char buf[BUFMIN];
    return asctime_r(tm, buf);
}

char *asctime_r(const struct tm *restrict tm, char *restrict buf)
{
    // locale = POSIX
    static const char *wday[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
    };
    static const char *mon[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };
    const char *wday_str =
        tm->tm_wday < 0 || tm->tm_wday > 7 ? "???" : wday[tm->tm_wday];
    const char *mon_str =
        tm->tm_mon < 0 || tm->tm_mon > 12 ? "???" : mon[tm->tm_mon];
    // FIXME: minus value may overflow the buffer size, asctime_r aborts
    // or report an error. use strftime to format instead
    // HACK: %.2d -> %02d is temporarily used
    snprintf(buf, BUFMIN, "%s %s %2d %02d:%02d:%02d %d\n", wday_str, mon_str,
             tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
             tm->tm_year + 1900);
    return buf;
}
