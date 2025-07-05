#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_pid_t
#define __NEED_sigset_t
#include <bits/alltypes.h>

int sigemptyset(sigset_t *__set);
int sigfillset(sigset_t *__set);
int sigaddset(sigset_t *__set, int __signum);
int sigdelset(sigset_t *__set, int __signum);
int sigismember(const sigset_t *__set, int __signum);

#define SIG_ERR  ((void (*)(int))-1)// 注册失败
#define SIG_DFL ((void (*)(int))0)  // 默认处理
#define SIG_IGN ((void (*)(int))1)  // 忽略信号

void (*signal(int __signum, void (*__handler)(int)))(int);

#define SA_NOCLDSTOP  1          // TODO 只关心子进程结束
#define SA_NOCLDWAIT  2          // TODO 不产生僵尸进程, 内核直接回收
#define SA_SIGINFO    4          // TODO 用拓展信号处理函数
#define SA_ONSTACK    0x08000000 // TODO 使用备用栈
#define SA_RESTART    0x10000000 // TODO 被打断的系统调用自动恢复执行
#define SA_NODEFER    0x40000000 // 不阻塞自身
#define SA_RESETHAND  0x80000000 // 一次性调用, 恢复 handler
#define SA_RESTORER   0x04000000 // TODO 使用用户提供的 restorer

struct sigaction {
	union {
		void (*sa_handler)(int);
		// void (*sa_sigaction)(int, siginfo_t *, void *);
	};
	sigset_t sa_mask;
	int sa_flags;
	void (*sa_restorer)(void);
};

int sigaction(int __signum, const struct sigaction *__act, struct sigaction *__oldact);

#define SIG_BLOCK     0
#define SIG_UNBLOCK   1
#define SIG_SETMASK   2

int sigprocmask(int __how, const sigset_t *__set, sigset_t *__oset);

int kill(pid_t __pid, int __sig);
int raise(int __sig);

#include <bits/signal.h>

/* os-specified */
int sigreturn();

__END_DECLS

#endif