#include <unistd.h>

#define ATEXIT_MAX 32 // POSIX.1

typedef void (*fn)();

static int cidx;
static fn calls[ATEXIT_MAX];

int atexit(void (*func)())
{
    if (cidx >= ATEXIT_MAX)
        return -1;
    calls[cidx++] = (fn)func;
    return 0;
}

void _Exit(int status)
{
    _exit(status);
}

void __libc_exit_fini();

void exit(int status)
{
    while (--cidx >= 0)
        calls[cidx]();
    __libc_exit_fini();
    _exit(status);
}
