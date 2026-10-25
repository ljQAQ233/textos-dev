#ifdef __wrapper

struct k_sigaction
{
    void (*handler)(int);
    unsigned long flags;
    void (*restorer)(void);
    unsigned mask[2];
};

    #include <signal.h>

static int w_sigaction(int signum, const struct sigaction *act,
                       struct sigaction *oldact)
{
    int ret;
    struct k_sigaction kold;
    if (act) {
        struct k_sigaction kact = {
            .handler = act->sa_handler,
            .flags = act->sa_flags,
            .restorer = act->sa_restorer,
            .mask = {[0] = (unsigned)act->sa_mask,
                     [1] = (unsigned)(act->sa_mask >> 32)},
        };
        ret = __w_syscall(13, signum, &kact, &kold, sizeof(sigset_t));
    } else {
        ret = __w_syscall(13, signum, 0, &kold, sizeof(sigset_t));
    }
    if (ret < 0) return ret;
    if (oldact) {
        oldact->sa_handler = kold.handler;
        oldact->sa_flags = kold.flags;
        oldact->sa_restorer = kold.restorer;
        oldact->sa_mask =
            kold.mask[0] | ((unsigned long long)kold.mask[1] << 32);
    }
}

static int w_sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    return __w_syscall(14, how, set, oset, sizeof(sigset_t));
}

static int w_sigsuspend(const sigset_t *sigmask)
{
    return __w_syscall(130, sigmask, sizeof(sigset_t));
}

#endif
