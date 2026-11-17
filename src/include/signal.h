#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

typedef int sig_atomic_t;
#define __NEED_pid_t
#define __NEED_sigset_t
#include <bits/alltypes.h>

struct sigaction;

int sigemptyset(sigset_t *__set);
int sigfillset(sigset_t *__set);
int sigaddset(sigset_t *__set, int __signum);
int sigdelset(sigset_t *__set, int __signum);
int sigismember(const sigset_t *__set, int __signum);

void (*signal(int __signum, void (*__handler)(int)))(int);
int sigaction(int __signum, const struct sigaction *__act,
              struct sigaction *__oldact);

#define SIG_BLOCK   0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

int sigprocmask(int __how, const sigset_t *__set, sigset_t *__oset);
int sigsuspend(const sigset_t *__sigmask);

int kill(pid_t __pid, int __sig);
int raise(int __sig);

#include <bits/signal.h>

/* os-specified */
int sigreturn();

__END_DECLS

#endif
