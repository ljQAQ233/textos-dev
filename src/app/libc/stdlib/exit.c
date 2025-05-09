#include <app/api.h>

#define ATEXIT_MAX 32 // POSIX.1

typedef void (*func_t)();

static int cidx;
static func_t calls[ATEXIT_MAX];

int atexit(void (*func)())
{
    calls[cidx++] = (func_t)func;
}

void _Exit(int status)
{
    _exit(status);
}

void exit(int status)
{
    while (--cidx >= 0)
        calls[cidx]();
    _exit(status);
}