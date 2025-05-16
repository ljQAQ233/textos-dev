#include <app/api.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    struct utsname u;
    if (uname(&u) < 0)
    {
        perror("uname");
        return 1;
    }

    printf("%s %s %s %s %s\n",
            u.sysname,
            u.nodename,
            u.release,
            u.version,
            u.machine);
    return 0;
}

