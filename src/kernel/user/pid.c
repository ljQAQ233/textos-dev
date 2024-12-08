#include <textos/task.h>
#include <textos/syscall.h>

RETVAL(int) sys_getpid()
{
    return task_current()->pid;
}

RETVAL(int) sys_getppid()
{
    return task_current()->ppid;
}

