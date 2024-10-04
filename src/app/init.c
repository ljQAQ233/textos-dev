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
    execve("/cat.elf", argv, envp);
    write(1, "execve failed!\n", 17);
    while(1);
}
