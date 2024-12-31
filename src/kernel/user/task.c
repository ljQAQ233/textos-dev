#include <textos/task.h>
#include <textos/syscall.h>

RETVAL(int) sys_fork()
{
    return task_fork();
}

RETVAL(void) sys_exit(int stat)
{
    task_exit(stat);
}

// uninterrupt
RETVAL(int) sys_wait4(int pid, int *stat, int opt, void *rusage)
{
    return task_wait(pid, stat, opt, rusage);
}

RETVAL(int) sys_getpid()
{
    return task_current()->pid;
}

RETVAL(int) sys_getppid()
{
    return task_current()->ppid;
}
