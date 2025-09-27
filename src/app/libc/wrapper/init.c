#include <string.h>
#include <sys/utsname.h>

int __is_linux;

int printf(char *, ...);

void __init_wrapper()
{
    char b[1 << 9];
    struct utsname *u = (void *)b;
    if (uname(u) < 0)
        return ;
    if (strcmp(u->sysname, "Linux") == 0)
        __is_linux = 1;
}
