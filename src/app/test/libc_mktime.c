#include <time.h>
#include <stdlib.h>
#include "unity/unity.h"

static struct test
{
    time_t stamp;
    int valid;
    struct tm tm;
} tests[] = {
#include "auto/libc_mktime.h"
#include "auto/libc_gmtime.h"
};

#define len(x) (sizeof(x) / sizeof(x[0]))

void libc_mktime()
{
    putenv("TZ=UTC");
    for (int i = 0; i < len(tests); i++) {
        if (!tests[i].valid) continue;
        time_t r = mktime(&tests[i].tm);
        time_t e = tests[i].stamp;
        TEST_ASSERT_EQUAL(e, r);
    }
}
//! register=libc_mktime
