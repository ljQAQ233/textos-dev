#include "time.h"
#include <limits.h>

static inline int sign(int x)
{
    return x < 0 ? -1 : 1;
}

// check if y overflows when adding x
static inline int chkadd(int y, int x)
{
    long long y_ = y - 1900;
    return (y_ > 0 && y_ > (long long)INT_MAX - x) ||
           (y_ < 0 && y_ < (long long)INT_MIN - x);
}

static inline int monthlen(int m, int y)
{
    m %= 12;
    if (m < 0) m += 12;
    if (m == 1 && isleap(y)) return 29;
    return mdays[m];
}

static int __norm_dmy(struct tm *tm, long long d)
{
    // mday -> mon -> year
    int y = tm->tm_year + 1900;
    int m = tm->tm_mon;
    for (;;) {
        if (m < 0 || m > 11) {
            int x = m / 12;
            if (m < 0) x -= 1;
            // check overflow
            if (chkadd(y - 1900, x)) return -1;
            y += x;
            m -= x * 12;
        }
        int end = monthlen(m, y);
        if (0 <= d && d < end) break;
        int direct = sign(d);
        m += direct;
        d -= direct * monthlen(m, y);
    }
    tm->tm_year = y - 1900;
    tm->tm_mon = m;
    tm->tm_mday = d + 1;
    return 0;
}

int __norm_time(struct tm *tm)
{
    // if data is incredibly wicked, e.g. every field reach the
    // limitation of `int`, it indeed overflows. I would prefer LL_t.
    // note that -> tm::tm_mday in [1, max]
    long long mi = tm->tm_min;
    long long h = tm->tm_hour;
    long long d = tm->tm_mday - 1;

    // sec -> min
    {
        int x = tm->tm_sec / 60;
        if (tm->tm_sec < 0) x -= 1;
        tm->tm_sec -= x * 60;
        mi += x;
    }

    // min -> hour
    {
        int x = mi / 60;
        if (mi < 0) x -= 1;
        mi -= x * 60;
        h += x;
    }

    // hour -> mday
    {
        int x = h / 24;
        if (h < 0) x -= 1;
        h -= x * 24;
        d += x;
    }
    tm->tm_min = mi;
    tm->tm_hour = h;
    return __norm_dmy(tm, d);
}
