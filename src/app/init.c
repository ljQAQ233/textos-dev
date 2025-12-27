#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>

char *argv[] = {
    "/bin/login",
    NULL,
};

char *envp[] = {
    NULL,
};

/*
 * run a shell on every tty
 *   - tty1
 *   - ttyS0
 */
int exec(char *tty)
{
    int fd = open(tty, O_RDWR);
    if (fd < 0)
        return -1;

    pid_t pid = fork();
    if (pid == 0)
    {
        dup2(fd, 1);
        dup2(fd, 2);
        execve(argv[0], argv, envp);
        write(1, "execve failed!\n", 17);
        while (1)
            syscall(SYS_yield);
    }
    close(fd);
    return 0;
}

void _start()
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

    // invoke page fault
    *((volatile int *)NULL);

    exec("/dev/tty1");
    exec("/dev/ttyS0");

    while (1)
        syscall(SYS_yield);
}
