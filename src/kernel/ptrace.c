#include <textos/ptrace.h>
#include <textos/errno.h>
#include <textos/syscall.h>
#include <textos/task.h>
#include <textos/signal.h>

#define IGN(x) (void)x

void ptrace_stop_self(int ev, task_t *tt)
{
    // irq disabled
    tt->retval = make_stat_stopped(SIGTRAP);
    task_report_wstat(tt);

    if (!tt->dbg_traced) return;
    if (tt->dbg_tracer) task_unblock(tt->dbg_tracer, 0);
    for (;;) {
        tt->dbg_waiting = true;
        int ret = task_block(tt, NULL, TASK_BLK, 0);
        tt->dbg_waiting = false;
        if (ret == 0) break;
        DEBUGK(K_ERROR, "traced task unblocked exceptionally - ret = %d\n",
               ret);
    }
}

void (*ptrace_evmap[32][2])(int, task_t *) = {
    [PTRACE_EVENT_EXEC] = {ptrace_stop_self, 0},
    [PTRACE_EVENT_SCENTRY] = {ptrace_stop_self, 0},
    [PTRACE_EVENT_SCEXIT] = {ptrace_stop_self, 0},
};

void ptrace_event(int ev)
{
    task_t *ttsk = task_current();
    if (!ttsk->dbg_traced) return;
    int opt = (ttsk->dbg_options >> ev) & 1;
    if (!ptrace_evmap[ev][opt]) return;
    ptrace_evmap[ev][opt](ev, ttsk);
}

#define DEF(name) long name(int req, pid_t pid, void *addr, void *data)

static DEF(forechk)
{
    // target task
    task_t *tsk = task_current();
    task_t *ttsk = task_get(pid);
    if (ttsk->ppid != tsk->pid) return -EPERM;
    if (!ttsk->dbg_traced) return -EPERM;
    return 0;
}

static DEF(stopchk)
{
    task_t *ttsk = task_get(pid);
    return ttsk->stat == TASK_BLK;
}

#define GOTO_IF_ERR(ret, expr, label) \
    if ((ret = (expr)) < 0) {         \
        goto label;                   \
    }

//
// ptrace operations
//

DEF(ptrace_traceme)
{
    IGN(pid);
    IGN(addr);
    IGN(data);
    task_t *tt = task_current();
    tt->dbg_traced = true;
    DEBUGK(K_TRACE, "task %d allows ptrace\n", tt->pid);
    return 0;
}

DEF(ptrace_syscall)
{
    long ret;
    GOTO_IF_ERR(ret, forechk(req, pid, addr, data), exit);
    GOTO_IF_ERR(ret, stopchk(req, pid, addr, data), exit);

    task_t *ttsk = task_get(pid);
    if (ttsk->stat != TASK_BLK) return -ESRCH;
    int newopt = PTRACE_EVENT_SCENTRY | PTRACE_EVENT_SCEXIT;
    ttsk->dbg_tracer = task_current();
    ttsk->dbg_options |= newopt;
    if (ttsk->stat == TASK_BLK && ttsk->dbg_waiting)
        task_unblock(ttsk, 0);
    ret = task_block(NULL, NULL, TASK_BLK, 0);
    ttsk->dbg_options &= ~newopt;
    ttsk->dbg_tracer = 0;
exit:
    return ret;
}

DEF(ptrace_getregs)
{
    long ret;
    GOTO_IF_ERR(ret, forechk(req, pid, addr, data), exit);
    GOTO_IF_ERR(ret, stopchk(req, pid, addr, data), exit);

    task_t *ttsk = task_get(pid);
    struct user_regs_struct *uregs = data;
    arch_ptrace_to_uregs(uregs, ttsk->sframe, ttsk->syscallno);
exit:
    return ret;
}

long (*ptrace_opmap[34])(int, pid_t, void *, void *) = {
    [PTRACE_TRACEME] = ptrace_traceme,
    [PTRACE_SYSCALL] = ptrace_syscall,
    [PTRACE_GETREGS] = ptrace_getregs,
};

__SYSCALL_DEFINE4(long, ptrace, int, req, pid_t, pid, void *, addr, void *,
                  data)
{
    long (*func)(int, pid_t, void *, void *) = 0;
    if (req < 0) return -EINVAL;
    if (req < 34) func = ptrace_opmap[req];
    if (func == 0) return -EINVAL;
    return func(req, pid, addr, data);
}
