#ifndef __SIGNAL_H__
#define __SIGNAL_H__

#include <bits/signal.h>

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);

typedef void (*sighandler_t)(int);
typedef struct sigaction sigaction_t;

#define SIG_ERR ((void (*)(int)) - 1)
#define SIG_DFL ((void (*)(int))0) // 默认处理
#define SIG_IGN ((void (*)(int))1) // 忽略信号

sighandler_t signal(int signum, sighandler_t handler);

int sigaction(int signum, const sigaction_t *act, sigaction_t *oldact);

int sigreturn();

#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

int kill(int pid, int sig);  // with permission check
int fkill(int pid, int sig); // without permisson check

// wstatus helpers
#define make_stat_exit(code)        (((code) & 0xff) << 8)
#define make_stat_signal(sig, core) (((sig) & 0x7f) | ((core) ? 0x80 : 0))
#define make_stat_stopped(sig)      ((((sig) & 0xff) << 8) | 0x7f)
#define make_stat_continued()       (0xffff)

// clang-format off
static inline const char *sig_to_str(int sig)
{
#define _(x) case SIG##x: return #x;
    switch (sig) {
    _(HUP) _(INT) _(QUIT) _(ILL) _(TRAP) _(ABRT) _(BUS) _(FPE) _(KILL)
    _(USR1) _(SEGV) _(USR2) _(PIPE) _(ALRM) _(TERM) _(STKFLT) _(CHLD)
    _(CONT) _(STOP) _(TSTP) _(TTIN) _(TTOU) _(URG) _(XCPU) _(XFSZ)
    _(VTALRM) _(PROF) _(WINCH) _(IO) _(PWR) _(SYS)
    default: return "UNKNOWN";
    }
#undef _
}
// clang-format on

#endif
