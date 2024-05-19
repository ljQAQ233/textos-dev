#ifndef __TIME_H__
#define __TIME_H__

typedef struct {
    u8  second;
    u8  minute;
    u8  hour;
    u8  day;
    u8  month;
    u32 year;
} time_t;

#endif
