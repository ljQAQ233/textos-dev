#include <signal.h>

int sigemptyset(sigset_t *set)
{
    if (!set) return -1;
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set)
{
    if (!set) return -1;
    *set = ~0ull;
    return 0;
}

int sigaddset(sigset_t *set, int signum)
{
    if (!set || signum <= 0 || signum > _NSIG) return -1;
    *set |= (1ull << (signum - 1));
    return 0;
}

int sigdelset(sigset_t *set, int signum)
{
    if (!set || signum <= 0 || signum > _NSIG) return -1;
    *set &= ~(1ull << (signum - 1));
    return 0;
}

int sigismember(const sigset_t *set, int signum)
{
    if (!set || signum <= 0 || signum > _NSIG) return -1;
    return ((*set & (1ull << (signum - 1))) != 0) ? 1 : 0;
}
