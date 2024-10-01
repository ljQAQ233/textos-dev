#include <app/sys.h>
#include <app/api.h>

void _start()
{
    int pid = fork();
    if (pid == 0)
    {
        while(1) write(1, "fork() -> child!\n", 17);
    }
    else
    {
        while(1) write(1, "fork() -> parent!\n", 18);
    }
    while(1);
}
