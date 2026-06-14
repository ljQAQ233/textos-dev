#include "stdio.h"
#include <errno.h>
#include <unistd.h>

// NOTE: ONLY TESTED ON LINUX
FILE *popen(const char *cmd, const char *type)
{
    FILE *f;
    int ppeer;
    int p[2];
    if (*type == 'r') {
        ppeer = 0;
    } else if (*type == 'w') {
        ppeer = 1;
    } else {
        errno = EINVAL;
        return 0;
    }
    if (pipe(p)) return 0;
    if (!(f = fdopen(p[ppeer], type))) {
        close(p[0]);
        close(p[1]);
        return 0;
    }

    // child's own end
    int cpeer = !ppeer;
    pid_t pid = fork();
    if (pid == 0) {
        close(p[ppeer]);
        close(cpeer);
        dup2(p[cpeer], cpeer);
        close(p[cpeer]);
        execl("/bin/sh", "sh", "-c", cmd, (char *)0);
        _exit(127);
    }
    close(p[cpeer]);

    f->peerpid = pid;
    return f;
}
