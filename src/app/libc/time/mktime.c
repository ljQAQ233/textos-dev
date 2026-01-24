#include <time.h>
#include <errno.h>

// non-leap year
static long long month[13] = {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31};

#define TS_MINUTE          (60)
#define TS_HOUR            (60 * 60)
#define TS_DAY             (60 * 60 * 24)
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define LEAP_CKR(year)     (year % 100 == 0 ? year % 400 == 0 : year % 4 == 0)

// SEE: docs/kernel/5.设备/048 时钟.md
time_t mktime(struct tm *tm)
{
    time_t Y = tm->tm_year + 1900;
    if (tm->tm_year < 70 || tm->tm_year > 9999 || tm->tm_mon < 0 ||
        tm->tm_mon > 11 || tm->tm_mday < 1 || tm->tm_mday > 31 ||
        tm->tm_hour < 0 || tm->tm_hour > 23 || tm->tm_min < 0 ||
        tm->tm_min > 59 || tm->tm_sec < 0 || tm->tm_sec > 60) {
        goto overflow;
    }
    // 计算闰年润过来的天数
    time_t offset = 0;
    offset += DIV_ROUND_UP(Y - 1972, 4);
    offset -= DIV_ROUND_UP(Y - 2000, 100);
    offset += DIV_ROUND_UP(Y - 2000, 400);
    // 全是平年的话, 度过的天数
    time_t common = (Y - 1970) * 365;
    // 计算在今年这天之前度过了多少天
    // tm_mon [0, 11] 0 是一月
    time_t days = month[tm->tm_mon];
    days += tm->tm_mday - 1;
    days += tm->tm_mon > 1 ? LEAP_CKR(Y) : 0;
    // 综上所述...
    time_t res = 0;
    res += (common + offset + days) * TS_DAY;
    res += tm->tm_hour * TS_HOUR;
    res += tm->tm_min * TS_MINUTE + tm->tm_sec;
    if (res < 0) {
        goto overflow;
    }
    return res;

overflow:
    errno = EOVERFLOW;
    return (time_t)-1;
}
