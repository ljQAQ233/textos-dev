#include <stdio.h>
#include <sys/mount.h>

int main(int argc, char const *argv[])
{
    if (argc != 2)
        return 1;

    char *dir = (char *)argv[1];
    if (umount(dir) < 0)
    {
        perror("umount");
        return 1;
    }

    return 0;
}
