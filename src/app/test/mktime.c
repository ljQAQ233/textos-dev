#include <time.h>
#include <stdio.h>
#include <stdlib.h>

struct test
{
    time_t stamp;
    int valid;
    struct tm tm;
} tests[] = {
    #include "auto/mktime.h"
    #include "auto/gmtime.h"
};

#define len(x) (sizeof(x) / sizeof(x[0]))

int main(int argc, char *argv[])
{
    putenv("TZ=UTC");
    for (int i = 0; i < len(tests); i++) {
        if (!tests[i].valid)
            continue;
        time_t r = mktime(&tests[i].tm);
        time_t e = tests[i].stamp;
        if (r != e) {
            printf("failed %d: \n", i);
            printf("  result %lld\n", (long long)r);
            printf("  expect %lld\n", (long long)e);
            return 1;
        }
    }
    printf("all test cases (%d) passed!\n", (int)len(tests));
    return 0;
}
