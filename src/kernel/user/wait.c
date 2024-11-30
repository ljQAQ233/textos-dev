#include <textos/task.h>
#include <textos/errno.h>
#include <textos/panic.h>
#include <textos/assert.h>

// uninterrupt
int sys_wait4(int pid, int *stat, int opt, void *rusage)
{
    return task_wait(pid, stat, opt, rusage);
}
