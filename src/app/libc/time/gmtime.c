#include "time.h"
#include <errno.h>
#include <limits.h>

struct tm *gmtime(const time_t *t)
{
    static struct tm tm;
    return gmtime_r(t, &tm);
}

/*
 * 将 时间戳 转换为 utc 时间, 这里将 t 按照不同粒度分解
 */
struct tm *gmtime_r(const time_t *restrict t, struct tm *restrict tm)
{
    time_t days = (*t - LEAPOCH) / TS_DAY;
    time_t inday = (*t - LEAPOCH) % TS_DAY;
    if (inday < 0) {
        /* 负数时间, 相应的 days 向 这天 对齐 */
        days -= 1;
        inday += TS_DAY;
    }
    time_t wday = (days + 3) % 7;
    if (wday < 0) wday += 7;

    /* 周期 (天) */
    static const time_t T4 = 365 * 4 + 1;
    static const time_t T100 = 365 * 100 + 24;
    static const time_t T400 = 365 * 400 + 97;
    time_t c4, c100, c400;
    c400 = days / T400;
    days %= T400;
    if (days < 0) {
        /*
         * 负数时间, 先将 400 周期往回拨, 这样之后的时间处理的
         * 是 400 年以内的, 可以直接在这个基准点上做加法
         */
        c400 -= 1;
        days += T400;
    }

    /*
     * 正常而言, 2000 开始的 400 年 每一个 100 年闰年数目是 25/24/24/24,
     * 这是由于算了 2000 年. 要是将 epoch 选到 "feb 30" 就可以 避开本年 (也就是
     * 400 闰年) 将 400 年这么划分: 24/24/24/25, 这就是 T100 里面 +24 的缘由啦
     * ૮(˶ᵔ ᵕ ᵔ˶)ა
     */
    c100 = days / T100;
    if (c100 == 4) {
        /* "25" 个闰年的那一个百年! 也就是: 特别地, 最后一年是闰年 */
        c100 -= 1;
    }
    days -= c100 * T100;

    c4 = days / T4;
    if (c4 == 25) {
        c4 -= 1;
    }
    days -= c4 * T4;

    /*
     * T4 以内的天数 以及 这年是不是闰年? y_rem == 0 OR 4 => 年份被 4 整除;
     * c100 == 0 => 被 400 整除; c4 != 0 => 在 T100 中间位置
     */
    time_t y_rem = days / 365;
    time_t leap = (!(y_rem % 4) && c4) || !c100;
    if (y_rem == 4) {
        y_rem -= 1;
    }
    days -= y_rem * 365;
    days += 31 + 28 + leap;
    if (days >= 365 + leap) {
        days -= 365 + leap;
        y_rem += 1;
    }
    time_t y = 100 + y_rem + 4 * c4 + 100 * c100 + 400 * c400;
    if (y > INT_MAX || y < INT_MIN) {
        goto overflow;
    }
    tm->tm_year = y;
    tm->tm_yday = days;
    tm->tm_wday = wday;

    /* 直接用 for 来计算月份, mdays 是平年每月天数 */
    int m = 0;
    for (; m < 12; m++) {
        int d = mdays[m];
        if (m == 1 && leap) d++;
        if (days < d) break;
        days -= d;
    }
    tm->tm_mon = m;
    tm->tm_mday = days + 1;

    /* 处理一天中的时间 */
    tm->tm_sec = inday % 60, inday /= 60;
    tm->tm_min = inday % 60, inday /= 60;
    tm->tm_hour = inday;

    /* utc 没有 dst */
    tm->tm_isdst = 0;
    tm->tm_gmtoff = 0;
    tm->tm_zone = "GMT";
    return tm;

overflow:
    errno = EOVERFLOW;
    return NULL;
}
