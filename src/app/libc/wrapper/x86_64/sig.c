#ifdef __wrapper

#include <signal.h>

static int w_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    return w_syscall(13, signum, act, oldact, sizeof(sigset_t));
}

#endif
