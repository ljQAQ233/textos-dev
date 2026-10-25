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
    // test code
    mkdir("/mnt", 0777);
    mount("/dev/hda1", "/mnt");
    mkdir("/mnt/dev", 0777);
    mknod("/mnt/dev/null", S_IFCHR | 0666, makedev(1, 3));
    mknod("/mnt/dev/zero", S_IFCHR | 0666, makedev(1, 5));
    mknod("/mnt/dev/full", S_IFCHR | 0666, makedev(1, 7));

    chown("/mnt/dev/null", 0, 0);
    chmod("/mnt/dev/null", 0000);

    // // invoke page fault
    // *((volatile int *)NULL);
    
    signal(SIGCHLD, sigchld);

    exec("/dev/tty1");
    exec("/dev/ttyS0");

    while (1)
        pause();
}
