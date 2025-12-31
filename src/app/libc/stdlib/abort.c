#include <signal.h>
#include <stdlib.h>

_Noreturn void abort()
{
    raise(SIGABRT);
    signal(SIGABRT, SIG_DFL);
    raise(SIGABRT);
    _Exit(127);
    __builtin_trap();
    __builtin_unreachable();
}
