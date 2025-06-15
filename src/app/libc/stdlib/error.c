/* __thread */ int errno;

#include <errno.h>

typedef struct {
    const int nr;
    const char *str;
} strerr_t;

#define E(x, y) { .nr = x, .str = y },

static strerr_t msg[] = {
    #include <bits/errstr.h>
};

static const char msg_def[] = "Unknown error";

char *__get_errstr(int nr)
{
    int s = 0, mid, cmp,
        t = sizeof(msg) / sizeof(strerr_t) - 1;
    while (s <= t) {
        mid = (s + t) / 2;
        cmp = msg[mid].nr;
        if (nr > cmp)
            s = mid + 1;
        else if (nr < cmp)
            t = mid - 1;
        else
            return (char *)msg[mid].str;
    }

    return (char *)msg_def;
}
