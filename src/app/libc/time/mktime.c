#include "time.h"
#include <errno.h>
#include <assert.h>

// SEE: docs/kernel/5.设备/048 时钟.md
// tm::tm_year 使用天文纪年, y = 0 -> BC1
time_t mktime(struct tm *tm)
{
    if (__norm_time(tm) < 0) goto overflow;
    time_t y = tm->tm_year + 1900;
    // 计算闰年润过来的天数
    time_t leap;
    time_t offset = leap_cnt(y, &leap);
    // 全是平年的话, 度过的天数
    time_t common = (y - 1970) * 365;
    // 计算在今年这天之前度过了多少天
    // tm_mon [0, 11] 0 是一月
    time_t days = s_mdays[tm->tm_mon];
    days += tm->tm_mday - 1;
    days += tm->tm_mon > 1 ? leap : 0;

    // 综上所述...
    time_t res = 0;
    res += (common + offset + days) * TS_DAY;
    res += tm->tm_hour * TS_HOUR;
    res += tm->tm_min * TS_MINUTE + tm->tm_sec;

    // 更新 wday 与 yday
    struct tm fixed;
    gmtime_r(&res, &fixed);
    assert(tm->tm_sec == fixed.tm_sec);
    assert(tm->tm_min == fixed.tm_min);
    assert(tm->tm_hour == fixed.tm_hour);
    assert(tm->tm_mday == fixed.tm_mday);
    assert(tm->tm_mon == fixed.tm_mon);
    assert(tm->tm_year == fixed.tm_year);

    tm->tm_yday = fixed.tm_yday;
    tm->tm_wday = fixed.tm_wday;
    return res;

overflow:
    errno = EOVERFLOW;
    return (time_t)-1;
}
