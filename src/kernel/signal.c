#include <textos/task.h>
#include <textos/errno.h>
#include <textos/signal.h>
#include <textos/syscall.h>
#include <textos/klib/string.h>

int sigemptyset(sigset_t *set)
{
    if (!set)
        return -1;
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set)
{
    if (!set)
        return -1;
    *set = ~0ull;
    return 0;
}

int sigaddset(sigset_t *set, int signum)
{
    if (!set || signum <= 0 || signum > _NSIG)
        return -1;
    *set |= (1ull << (signum - 1));
    return 0;
}

int sigdelset(sigset_t *set, int signum)
{
    if (!set || signum <= 0 || signum > _NSIG)
        return -1;
    *set &= ~(1ull << (signum - 1));
    return 0;
}

int sigismember(const sigset_t *set, int signum)
{
    if (!set || signum <= 0 || signum > _NSIG)
        return -1;
    return ((*set & (1ull << (signum - 1))) != 0) ? 1 : 0;
}

/*
 * task signal
*/

static void sig_core(int sig)
{
    // NOTE: coredump not supported yet
    task_exit(make_stat_signal(sig, 0));
}

static void sig_term(int sig)
{
    task_exit(make_stat_signal(sig, 0));
}

static void sig_ignore(int sig)
{
}

static void sig_stop(int sig)
{
}

static void sig_cont(int sig)
{
}

static sighandler_t sig_def[] = {
    [SIGHUP]    = sig_term,
    [SIGINT]    = sig_term,
    [SIGQUIT]   = sig_core,
    [SIGILL]    = sig_core,
    [SIGTRAP]   = sig_core,
    [SIGABRT]   = sig_core,
    [SIGBUS]    = sig_core,
    [SIGFPE]    = sig_core,
    [SIGKILL]   = NULL,
    [SIGUSR1]   = sig_term,
    [SIGSEGV]   = sig_core,
    [SIGUSR2]   = sig_term,
    [SIGPIPE]   = sig_term,
    [SIGALRM]   = sig_term,
    [SIGTERM]   = sig_term,
    [SIGCHLD]   = sig_ignore,
    [SIGCONT]   = sig_cont,
    [SIGSTOP]   = NULL,
    [SIGTSTP]   = sig_stop,
    [SIGTTIN]   = sig_stop,
    [SIGTTOU]   = sig_stop,
    [SIGURG]    = sig_ignore,
    [SIGXCPU]   = sig_core,
    [SIGXFSZ]   = sig_core,
    [SIGVTALRM] = sig_term,
    [SIGPROF]   = sig_term,
    [SIGWINCH]  = sig_ignore,
    [SIGIO]     = sig_ignore,
    [SIGPWR]    = sig_term,
    [SIGSYS]    = sig_core,
};

/**
 * @brief handle pending signals of this task
 *
 * @param iframe interrupt context
 * @param nr syscall number if called from syscall, or -1 if from interrupts
 */
void __handle_signal(intr_frame_t *iframe, int nr)
{
    task_t *tsk = task_current();
    sigset_t *sig = &tsk->sigpend;

    *sig &= ~tsk->sigmask;
    if (*sig == 0) return;
    if (sigismember(sig, SIGKILL)) task_exit(make_stat_signal(SIGKILL, 0));

    for (int i = 1 ; i <= _NSIG ; i++) {
        if (!sigismember(sig, i)) continue;
        sigdelset(sig, i);

        sigaction_t *act = &tsk->sigacts[i-1];
        sighandler_t handler = act->sa_handler;
        sigset_t blk = act->sa_mask;
        if (handler == SIG_IGN)
            return;
        else if (handler == SIG_DFL) {
            sig_def[i](i);
            return;
        }

        if (act->sa_flags & SA_RESETHAND) act->sa_handler = SIG_DFL;
        if (~act->sa_flags & SA_NODEFER) sigaddset(&blk, i);

        struct signal_ffl ffl = {
            .sysret = 0,
            .syscall = -1,
        };
        if (nr >= 0) {
            ffl.sysret = 1;
            if (tsk->sframe->rax == -EINTR && (act->sa_flags & SA_RESTART))
                ffl.syscall = nr;
        }

        /* save stack frame - note that we must keep a region for compiler
         * to store variables, i.e.  the red zone (128 bytes) */
        signal_frame_t *frame = (void *)iframe->rsp - sizeof(signal_frame_t) - 128;
        frame->restorer = act->sa_restorer;
        frame->signum = i;
        frame->sigprv = tsk->sigcurr;
        frame->sigmask = tsk->sigmask;
        frame->sigffl = ffl;
        frame->r15 = iframe->r15;
        frame->r14 = iframe->r14;
        frame->r13 = iframe->r13;
        frame->r12 = iframe->r12;
        frame->r11 = iframe->r11;
        frame->r10 = iframe->r10;
        frame->r9 = iframe->r9;
        frame->r8 = iframe->r8;
        frame->rdi = iframe->rdi;
        frame->rsi = iframe->rsi;
        frame->rbp = iframe->rbp;
        frame->rdx = iframe->rdx;
        frame->rcx = iframe->rcx;
        frame->rbx = iframe->rbx;
        frame->rax = iframe->rax;
        frame->vector = iframe->vector;
        frame->errcode = iframe->errcode;
        frame->rip = iframe->rip;
        frame->cs = iframe->cs;
        frame->rflags = iframe->rflags;
        frame->rsp = iframe->rsp;
        frame->ss = iframe->ss;

        iframe->rdi = i;
        iframe->rip = (addr_t)handler;
        iframe->rsp = (addr_t)frame;

        tsk->sigcurr = i;
        tsk->sigmask |= blk;
        break;
    }
}

#include <gdt.h>
#include <textos/panic.h>

/*
 * restorer 调用之后来到这里, 此时 rsp 指向 signal_frame_t::restorer 的
 * 下面. sigreturn 对 frame 进行检查, 这里只是简单的验证了几个必要值
 */
__SYSCALL_DEFINE0(int, sigreturn)
{
    task_t *tsk = task_current();
    intr_frame_t *sframe = tsk->sframe;
    /*
     * signal handler 最后的 ret 会调用 restorer, 有点像 ROP 攻击...
     * 所以 rsp 刚好指向 restorer 之后, 故 -8 获得原来的 信号帧.
     */
    signal_frame_t *frame = (void *)sframe->rsp - 8;

    if (frame->signum != tsk->sigcurr)
        PANIC("signum error\n");
    if (frame->cs != ((USER_CODE_SEG << 3) | 3) ||
        frame->ss != ((USER_DATA_SEG << 3) | 3))
        PANIC("ss / cs error\n");
    if (~frame->rflags & (1 << 9))
        PANIC("eflags error\n");

    tsk->sigcurr = frame->sigprv;
    tsk->sigmask = frame->sigmask;

    addr_t rsp = (addr_t)&frame->r15;
    void *return_ip = 0;
    intr_frame_t *iframe = (intr_frame_t *)rsp;
    if (frame->sigffl.syscall >= 0) {
        /*
         * re-invoke syscall instruction with the initial arg0,
         * which take up 2 bytes on x86 platforms.
         */
        iframe->rip -= 2;
        iframe->rax = frame->sigffl.syscall;
    }
    if (frame->sigffl.sysret)
        return_ip = (void *)msyscall_exit;
    else
        return_ip = (void *)intr_exit;
    __asm__ volatile( // reset stack
        "movq %0, %%rsp\n"
        "jmp *%1\n"
        :
        : "r"(rsp), "r"(return_ip)
        : "memory");
    __builtin_unreachable();
}

__SYSCALL_DEFINE3(int, sigaction, int, signum, const sigaction_t *, act, sigaction_t *, oldact)
{
    task_t *tsk = task_current();
    if (signum <= 0 || signum >= _NSIG) return -EINVAL;
    if (signum == SIGKILL || signum == SIGSTOP) return -EINVAL;
    if (oldact) {
        memcpy(oldact, &tsk->sigacts[signum - 1], sizeof(sigaction_t));
    }
    if (act) {
        sigaction_t *newact = &tsk->sigacts[signum - 1];
        newact->sa_handler = act->sa_handler;
        newact->sa_flags = act->sa_flags;
        newact->sa_restorer = act->sa_restorer;
        newact->sa_mask = act->sa_mask;
    }
    return 0;
}

__SYSCALL_DEFINE3(int, sigprocmask, int, how, const sigset_t *, set, sigset_t *, oset)
{
    task_t *tsk = task_current();
    if (oset) *oset = tsk->sigmask;
    if (!set) return 0;

    switch (how) {
        case SIG_BLOCK:
            tsk->sigmask |= *set;
        case SIG_UNBLOCK:
            tsk->sigmask &= ~*set;
        case SIG_SETMASK:
            tsk->sigmask = *set;
            break;
    }
    return 0;
}

static void notify_parent(task_t *tsk)
{
    kill(tsk->ppid, SIGCHLD);
}

__SYSCALL_DEFINE2(int, kill, int, pid, int, sig)
{
    if (sig < 0 || sig >= _NSIG)
        return -EINVAL;

    if (pid <= 0)
    {
        if (pid == 0)
            pid = -task_current()->pgid;
        int found = 0;
        int killed = 0;
        for (int i = 1 ; i < TASK_MAX ; i++)
        {
            if (!table[i] || table[i]->pgid != -pid)
                continue;
            found = 1;
            if (kill(table[i]->pid, sig) < 0)
                continue;
            killed = 1;
        }
        if (!found)
            return -ESRCH;
        if (!killed)
            return -EPERM;
        return 0;
    }

    task_t *tsk = task_current();
    task_t *ptsk = task_get(pid);
    if (!ptsk)
        return -ESRCH;
    if (tsk->euid != 0 &&
        tsk->euid != ptsk->ruid &&
        tsk->euid != ptsk->suid &&
        tsk->ruid != ptsk->ruid &&
        tsk->ruid != ptsk->suid)
        return -EPERM;
    
    /*
     * If sig is 0 (the  null  signal), error checking is performed but no signal
     * is actually sent. The null signal can be used to check the validity of pid.
     * If sig is SIGKILL, it's necessary to wait for its exit from a syscall, i.e.
     * release system resources. Handle it in `__handle_signal`!
     */
    if (sig == 0)
        return 0;
    sigaddset(&ptsk->sigpend, sig);
    DEBUGK(K_TRACE, "signal %d sent to pid=%d well\n", sig, ptsk->pid);

    if (sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU) {
        if (ptsk->stat == TASK_PRE || ptsk->stat == TASK_RUN ||
            ptsk->stat == TASK_SLP || ptsk->stat == TASK_BLK) {
            ptsk->stat_before_stop = ptsk->stat;
            ptsk->stat = TASK_STP;
            ptsk->retval = make_stat_stopped(sig);
            notify_parent(ptsk);
            if (ptsk == tsk) task_yield();
        }
        return 0;
    }
    if (sig == SIGKILL) {
        int status = make_stat_signal(SIGKILL, 0);
        /*
         * wake up the proc, stimulating it to release the resources it holds.
         * after releasing, before escape from kernel space, SIGKILL works.
         */
        if (ptsk->stat == TASK_SLP || ptsk->stat == TASK_BLK)
            task_unblock(ptsk, status);
        if (ptsk->stat == TASK_STP)
            ptsk->stat = TASK_PRE;
        return 0;
    }
    if (sig == SIGCONT)
    {
        if (ptsk->stat == TASK_STP) {
            ptsk->stat = ptsk->stat_before_stop;
            ptsk->retval = make_stat_continued();
            notify_parent(ptsk);
        }
        return 0;
    }

    /*
     * wake up it if it is blocked, forcing it to handle the signal
     * send to it. a syscall (if-so) might return EINTR.
     * TODO: is it interruptible?
     */
    if (ptsk->stat == TASK_BLK || ptsk->stat == TASK_SLP) {
        task_unblock(ptsk, -EINTR);
    }

    return 0;
}
