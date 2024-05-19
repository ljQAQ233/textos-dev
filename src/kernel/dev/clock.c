/* RTC driver */
#include <io.h>
#include <string.h>
#include <textos/printk.h>
#include <textos/klib/time.h>

#define R_SECOND   0x00
#define R_MINUTE   0x02
#define R_HOUR     0x04
#define R_WEEKDAY  0x06
#define R_DAY      0x07
#define R_MONTH    0x08
#define R_YEAR     0x09
#define R_STAT_A   0x0A
#define R_STAT_B   0x0B
#define R_CENTURY  0x32 // maybe

#define STAT_NOBCD   (1 << 3)

#define R_CMOS_ADDR 0x70 // the port to address register which will be written or read
#define R_CMOS_DATA 0x71 // the port to write or read data

static inline u8 read_rtc (u16 reg)
{
    /* Close the NMI with the flag 0x80 */
    outb (R_CMOS_ADDR, reg | (1 << 7));
    return inb (R_CMOS_DATA);
}

static u64 __startup_time;

#define DUMP_BCD(bcd) \
    ((bcd >> 4) * 10 + (bcd & 0xF))

#define CENTURY_DEFAULT (21)

void clock_init ()
{
    time_t tm;
    memset (&tm, 0, sizeof(time_t));

    /* 让 CMOS 自己告诉我们它有没有用 BCD */
    bool BcdMode = !(read_rtc(R_STAT_B) & STAT_NOBCD);

    /* 在获取时间时, 尽量把容易改变的放在最后 */
    u8 Century;
    if (false) { // TODO
        Century = read_rtc (R_CENTURY);
        if (BcdMode)
          Century = DUMP_BCD(Century);
    } else {
        Century = CENTURY_DEFAULT;
    } 

    tm.year    = read_rtc (R_YEAR);
    tm.month   = read_rtc (R_MONTH);
    tm.day     = read_rtc (R_DAY);
    tm.hour    = read_rtc (R_HOUR);
    tm.minute  = read_rtc (R_MINUTE);
    tm.second  = read_rtc (R_SECOND);
    
    if (BcdMode)
    {
        tm.year    = DUMP_BCD(tm.year);
        tm.month   = DUMP_BCD(tm.month);
        tm.day     = DUMP_BCD(tm.day);
        tm.hour    = DUMP_BCD(tm.hour);
        tm.minute  = DUMP_BCD(tm.minute);
        tm.second  = DUMP_BCD(tm.second);
    }
    tm.year += 100 * (Century - 1);

    u64 stamp = time_stamp (&tm);
    printk ("time now -> %u/%u/%u %02u:%02u:%02u (%llu)\n",
           tm.year, tm.month, tm.day,
           tm.hour, tm.minute, tm.second, stamp);
}

