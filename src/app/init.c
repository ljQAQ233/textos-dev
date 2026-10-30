#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>

int exec(char *tty)
{
    pid_t pid = fork();
    if (pid == 0) {
        char *argv[] = {
            "/bin/getty",
            tty,
            "linux",
            NULL,
        };
        execve(argv[0], argv, NULL);
        exit(1);
    }
    return 0;
}

extern int dprintf(int, char *, ...);

void sigchld(int sig)
{
    pid_t pid;
    while ((pid = wait4(-1, 0, WEXITED | WNOHANG, 0) > 0)) {
        (void)pid;
    }
}

int main()
{
    signal(SIGCHLD, sigchld);

    exec("/dev/tty1");
    exec("/dev/ttyS0");

    while (1)
        pause();
}
