#include <textos/task.h>
#include <textos/errno.h>
#include <textos/signal.h>
#include <textos/syscall.h>

#include <string.h>

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
    [SIGCONT]   = sig_ignore,
    [SIGSTOP]   = NULL,
    [SIGTSTP]   = sig_term,
    [SIGTTIN]   = sig_term,
    [SIGTTOU]   = sig_term,
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

void __handle_signal(intr_frame_t *iframe)
{
    task_t *tsk = task_current();
    sigset_t *sig = &tsk->sigpend;

    *sig &= ~tsk->sigmask;
    if (*sig == 0)
        return ;
    if (!sigismember(sig, SIGKILL))
        task_exit(make_stat_signal(SIGKILL, 0));

    for (int i = 1 ; i <= _NSIG ; i++) {
        if (!sigismember(sig, i))
            continue;
        sigdelset(sig, i);

        sigaction_t *act = &tsk->sigacts[i-1];
        sighandler_t handler = act->sa_handler;
        if (handler == SIG_IGN)
            return ;
        else if (handler == SIG_DFL) {
            sig_def[i](i);
            return ;
        }

        if (act->sa_flags & SA_RESETHAND)
            act->sa_handler = SIG_DFL;

        sigset_t blk = act->sa_mask;
        if (~act->sa_flags & SA_NODEFER)
            sigaddset(&blk, i);

        signal_frame_t *frame = (void *)iframe->rsp - sizeof(signal_frame_t) - 128;

        frame->restorer = act->sa_restorer;
        frame->signum = i;
        frame->sigprv = tsk->sigcurr;
        frame->sigmask = tsk->sigmask;
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
    __asm__ volatile(
        "movq %0, %%rsp\n"
        "jmp intr_exit\n"
        :
        : "r" (rsp)
        : "memory"
    );

    __builtin_unreachable();
}

__SYSCALL_DEFINE3(int, sigaction, int, signum, const sigaction_t *, act, sigaction_t *, oldact)
{
    task_t *tsk = task_current();
    if (signum <= 0 || signum >= _NSIG)
        return -EINVAL;
    if (signum == SIGKILL || signum == SIGSTOP)
        return -EINVAL;
    if (!act)
        return -EINVAL;

    if (oldact)
        memcpy(oldact, &tsk->sigacts[signum-1], sizeof(sigaction_t));

    sigaction_t *newact = &tsk->sigacts[signum-1];
    newact->sa_handler = act->sa_handler;
    newact->sa_flags = act->sa_flags;
    newact->sa_restorer = act->sa_restorer;
    newact->sa_mask = act->sa_mask;
    return 0;
}

__SYSCALL_DEFINE3(int, sigprocmask, int, how, const sigset_t *, set, sigset_t *, oset)
{
    task_t *tsk = task_current();
    if (oset)
        *oset = tsk->sigmask;

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
    if (sig == SIGSTOP)
    {
        ptsk->stat = TASK_STP;
        if (ptsk == tsk)
            task_yield();
        return 0;
    }
    if (sig == SIGKILL)
    {
        int status = make_stat_signal(SIGKILL, 0);
        if (ptsk->stat == TASK_SLP || ptsk->stat == TASK_BLK)
            task_unblock(ptsk, status);
        if (ptsk->stat == TASK_STP)
            ptsk->stat = TASK_PRE;
    }
    if (sig == SIGCONT)
    {
        if (ptsk->stat == TASK_STP)
            ptsk->stat = TASK_PRE;
    }

    return 0;
}
