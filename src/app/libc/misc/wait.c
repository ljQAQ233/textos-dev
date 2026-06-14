// impl for sys/wait.h
#include <sys/wait.h>

pid_t wait(int *stat)
{
    return waitpid(-1, stat, 0);
}

pid_t waitpid(pid_t pid, int *stat, int opt)
{
    return wait4(pid, stat, opt, 0);
}
