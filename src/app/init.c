#include <app/sys.h>
#include <app/api.h>

#include <stdio.h>
#include <math.h>

char buf[64];

void _start()
{
    char *argv[] = {
        NULL,
    };
    char *envp[] = {
        "PWD=/",
        NULL,
    };

    int fd = open("/config", O_RDONLY);
    perror(NULL);

    double x = cos(M_PI / 3);
    x = x * 7;
    x = x / 0;
    x = x + 3;

    /*
    int pid = fork();
    if (pid == 0) {
        execve("/cat.elf", argv, envp);
    } else {
        int stat;
        int pchd = wait(&stat);
        printf("chd %d exited : %d\n", pchd, stat);
        while(1);
    }
    */

    mknod("/zero", 0, makedev(0, 0));
    execve("/sh.elf", argv, envp);

    write(1, "execve failed!\n", 17);
    while(1);
}
