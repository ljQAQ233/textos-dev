#include <time.h>
#include <string.h>
#include "unity/unity.h"

// test pad-needed formats
struct tm tm0 = {
    .tm_year = 108,
    .tm_mon = 2,
    .tm_mday = 4,
    .tm_hour = 6,
    .tm_min = 7,
    .tm_sec = 8,
    .tm_wday = 2,
    .tm_yday = 63,
    .tm_isdst = 0,
};

struct tm tm1 = {
    .tm_sec = 38,
    .tm_min = 46,
    .tm_hour = 21,
    .tm_mday = 18,
    .tm_mon = 10,
    .tm_year = 432,
    .tm_wday = 5,
    .tm_yday = 322,
    .tm_isdst = 0,
};

char buf[100];

#define tostr(fmt, tm) strftime(buf, sizeof(buf), fmt, tm)
#define equ(tm, fmt1, fmt2)                                                 \
    do {                                                                    \
        int __ret;                                                          \
        char buf1[100];                                                     \
        __ret = tostr(fmt2, &tm);                                           \
        TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(__ret, 0,                         \
                                          "zero returned when using fmt2"); \
        strcpy(buf1, buf);                                                  \
        __ret = tostr(fmt1, &tm);                                           \
        TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(__ret, 0,                         \
                                          "zero returned when using fmt1"); \
        TEST_ASSERT_EQUAL_STRING(buf, buf1);                                \
    } while (0);

#define test(tm, fmt, expect)                                         \
    do {                                                              \
        int __ret = tostr(fmt, &tm);                                  \
        TEST_ASSERT_NOT_EQUAL_INT_MESSAGE(0, __ret, "zero returned"); \
        TEST_ASSERT_EQUAL_STRING(expect, buf);                        \
        TEST_ASSERT_EQUAL_INT_MESSAGE(sizeof(expect) - 1, __ret,      \
                                      "returned count not matched");  \
    } while (0)

void libc_strftime()
{
    // %a
    // %A
    // %b
    // %B
    // %c

    test(tm0, "%C", "20");
    test(tm1, "%C", "23");

    test(tm0, "%d", "04");
    test(tm1, "%d", "18");

    equ(tm1, "%D", "%m/%d/%y");

    test(tm0, "%e", " 4");
    test(tm1, "%e", "18");

    equ(tm1, "%F", "%Y-%m-%d");

    // %g
    // %G
    // equ("%h", "%b", tm1)
    test(tm1, "%H", "21");
    // %I
    // %j
    test(tm0, "%m", "02");
    test(tm1, "%m", "10");
    // %M
    // %n
    test(tm0, "%n", "\n");
    // %p
    // %r
    equ(tm0, "%R", "%H:%M");
    equ(tm1, "%R", "%H:%M");

    test(tm0, "%S", "08");
    test(tm1, "%S", "38");

    test(tm1, "%t", "\t");

    equ(tm0, "%T", "%H:%M:%S");
    equ(tm1, "%T", "%H:%M:%S");

    test(tm0, "%u", "2");
    test(tm1, "%u", "5");
    // %U
    // %V
    // %w
    // %W
    // %x
    // %X
    test(tm0, "%y", "08");
    test(tm1, "%y", "32");

    test(tm0, "%Y", "2008");
    test(tm1, "%Y", "2332");
    // %z
    // %Z

    test(tm1, "%%", "%");
}

// !register=libc_strftime
