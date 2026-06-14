#include "stdio.h"
#include <sys/wait.h>
#include <errno.h>

int pclose(FILE *f)
{
    int pid = f->peerpid;
    int ws;
    fclose(f);

retry:
    if (waitpid(pid, &ws, 0) < 0) {
        if (errno == EINTR) goto retry;
        return -1;
    }
    return ws;
}
