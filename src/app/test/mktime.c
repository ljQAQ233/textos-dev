#include <time.h>
#include <stdio.h>
#include <stdlib.h>

struct test
{
    time_t stamp;
    struct tm tm;
} tests[] = {
#ifdef __GNUC__
    #if __has_include("auto/mktime.h")
        #include "auto/mktime.h"
    #endif
#endif
    {0, {0, 0, 0, 1, 0, 70, 4, 0, 0}},
    {23333, {53, 28, 6, 1, 0, 70, 4, 0, 0}},
    {114514, {34, 48, 7, 2, 0, 70, 5, 1, 0}},
};

#define len(x) (sizeof(x) / sizeof(x[0]))

int main(int argc, char *argv[])
{
    putenv("TZ=UTC");
    for (int i = 0; i < len(tests); i++) {
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
