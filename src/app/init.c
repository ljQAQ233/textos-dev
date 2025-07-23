#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/sysmacros.h>

void _start()
{
    char *argv[] = {
        NULL,
    };
    char *envp[] = {
        "PWD=/",
        NULL,
    };

    int fd = open("/dev/tty1", O_RDWR);
    dup2(fd, 1);
    dup2(fd, 2);

    // test code
    mkdir("/mnt", 0777);
    mount("/dev/hda1", "/mnt");
    mkdir("/mnt/dev", 0777);
    mknod("/mnt/dev/null", S_IFCHR | 0666, makedev(1, 3));
    mknod("/mnt/dev/zero", S_IFCHR | 0666, makedev(1, 5));
    mknod("/mnt/dev/full", S_IFCHR | 0666, makedev(1, 7));

    chown("/mnt/dev/null", 0, 0);
    chmod("/mnt/dev/null", 0000);

    execve("/bin/sh", argv, envp);
    write(1, "execve failed!\n", 17);
    while(1);
}
