#include <app/api.h>

void _start()
{
    char *argv[] = {
        NULL,
    };
    char *envp[] = {
        "PWD=/",
        NULL,
    };

    execve("/bin/sh", argv, envp);
    write(1, "execve failed!\n", 17);
    while(1);
}
