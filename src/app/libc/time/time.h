#include <time.h>

#define TS_MINUTE (60)
#define TS_HOUR   (60 * 60)
#define TS_DAY    (60 * 60 * 24)

/* Wed Mar  1 12:00:00 AM UTC 2000 */
#define LEAPOCH (951868800)

int __norm_time(struct tm *tm);

// y 是否是闰年, y >= 0
static inline int isleap(int y)
{
    return y % 100 == 0 ? y % 400 == 0 : y % 4 == 0;
}

// 计算 [1, y] 之间有多少个闰年
static inline int leap_1(int y)
{
    return y / 4 - y / 100 + y / 400;
}

static inline time_t leap_cnt(int y, time_t *leap)
{
    if (y > 0) {
        *leap = isleap(y);
        return leap_1(y - 1) - leap_1(1969);
    }
    *leap = isleap(-y);
    time_t cnt = 0;
    cnt += -leap_1(-y);   // 映射到正数年
    cnt += !y ? 0 : -1;   // BC1 过去否?
    cnt += -leap_1(1969); // 加上 AD 的部分
    return cnt;
}

static const int mdays[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

// sum of days in a month for a non-leap year
static const int s_mdays[13] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365,
};
