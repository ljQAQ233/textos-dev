#include <textos/task.h>

int sys_getpid()
{
    return task_current()->pid;
}

int sys_getppid()
{
    return task_current()->ppid;
}

