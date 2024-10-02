#include <app/sys.h>
#include <app/api.h>

void _start()
{
    char *argv[] = {
        "TECH",
        "OTAKUS",
        "SAVE",
        "THE",
        "WORLD",
        NULL,
    };
    char *envp[] = {
        "PWD=/",
        NULL,
    };
    execve("/echo.elf", argv, envp);
    write(1, "execve failed!\n", 17);
    while(1);
}
