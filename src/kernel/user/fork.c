#include <textos/task.h>
#include <textos/syscall.h>

RETVAL(int) sys_fork()
{
    return task_fork();
}
