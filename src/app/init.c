#include <app/sys.h>
#include <app/api.h>

void _start()
{
    char *argv[] = {
        "/config.ini",
        NULL,
    };
    char *envp[] = {
        "PWD=/",
        NULL,
    };

    int fd = open("/cat.txt", O_RDWR | O_CREAT);
    close(1);
    dup2(fd, 1);

    execve("/cat.elf", argv, envp);
    write(1, "execve failed!\n", 17);
    while(1);
}
