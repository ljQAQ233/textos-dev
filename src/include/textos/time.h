#ifndef __TIME_H__
#define __TIME_H__

typedef struct {
    u8  second;
    u8  minute;
    u8  hour;
    u8  day;
    u8  month;
    u32 year;
} rtc_tm_t;

typedef int64 time_t;

time_t arch_time_now();

#endif
