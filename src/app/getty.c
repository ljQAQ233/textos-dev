#include <assert.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "util.h"

PROG_NAME("getty");

int getty(const char *path)
{
    char *argv[] = {
        "/bin/login",
        NULL,
    };

    pid_t sid = setsid();
    if (sid < 0)
        if (getsid(0) != getpid()) goto err;

    // take over tty as our controlling terminal
    int fd = open(path, O_RDWR);
    if (fd < 0) goto err;
    if (fd != 0) goto err;

    dup2(fd, 1);
    dup2(fd, 2);
    execve(argv[0], argv, NULL);
    write(1, "execve failed!\n", 17);
    while (1)
        pause();
err:
    error();
    return 1;
}

int main(int argc, char *argv[])
{
    assert(argc >= 2);
    return getty(argv[1]);
}
