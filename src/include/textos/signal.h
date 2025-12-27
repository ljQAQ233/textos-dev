#ifndef __SIGNAL_H__
#define __SIGNAL_H__

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sigaddset(sigset_t *set, int signum);
int sigdelset(sigset_t *set, int signum);
int sigismember(const sigset_t *set, int signum);

typedef void (*sighandler_t)(int);

#define SIG_ERR  ((void (*)(int))-1)
#define SIG_DFL ((void (*)(int))0)  // 默认处理
#define SIG_IGN ((void (*)(int))1)  // 忽略信号

sighandler_t signal(int signum, sighandler_t handler);

typedef struct sigaction
{
    union {
        void (*sa_handler)(int);
    };
    unsigned long sa_flags;
    void (*sa_restorer)(void);
    sigset_t sa_mask;
} sigaction_t;

int sigaction(int signum, const sigaction_t *act, sigaction_t *oldact);

int sigreturn();

#define SIG_BLOCK     0
#define SIG_UNBLOCK   1
#define SIG_SETMASK   2

int sigprocmask(int how, const sigset_t *set, sigset_t *oset);

int kill(int pid, int sig);

#define SA_NOCLDSTOP  1          // TODO 只关心子进程结束
#define SA_NOCLDWAIT  2          // TODO 不产生僵尸进程, 内核直接回收
#define SA_SIGINFO    4          // TODO 用拓展信号处理函数
#define SA_ONSTACK    0x08000000 // TODO 使用备用栈
#define SA_RESTART    0x10000000 // TODO 被打断的系统调用自动恢复执行
#define SA_NODEFER    0x40000000 // 不阻塞自身
#define SA_RESETHAND  0x80000000 // 一次性调用, 恢复 handler
#define SA_RESTORER   0x04000000 // TODO 使用用户提供的 restorer

#include <bits/signal.h>

#define make_stat_exit(code) (((code) & 0xff) << 8)
#define make_stat_signal(sig, core) (((sig) & 0x7f) | ((core) ? 0x80 : 0))
#define make_stat_stopped(sig) (((sig) & 0xff) << 8)
#define make_stat_continued() (0xffff)

#endif
