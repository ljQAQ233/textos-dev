#include <time.h>
#include <stdio.h>
#include <stdlib.h>

struct test
{
    time_t stamp;
    int valid;
    struct tm tm;
} tests[] = {
    #include "auto/gmtime.h"
};

#define len(x) (sizeof(x) / sizeof(x[0]))

static inline int eq(const struct tm *t1, const struct tm *t2)
{
    return t1->tm_year == t2->tm_year && t1->tm_mon == t2->tm_mon &&
           t1->tm_mday == t2->tm_mday && t1->tm_hour == t2->tm_hour &&
           t1->tm_min == t2->tm_min && t1->tm_sec == t2->tm_sec;
}

int main(int argc, char *argv[])
{
    putenv("TZ=UTC");
    for (int i = 0; i < len(tests); i++) {
        struct tm *r = gmtime(&tests[i].stamp);
        struct tm *e = &tests[i].tm;
        if (r == NULL && !tests[i].valid)
            continue;
        if (!eq(r, e)) {
            printf("failed %d: \n", i);
            printf("  result %s", asctime(r));
            printf("  expect %s", asctime(e));
            return 1;
        }
    }
    printf("all test cases (%d) passed!\n", (int)len(tests));
    return 0;
}
