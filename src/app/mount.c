#include <app/api.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    if (argc != 3)
        return 1;

    char *src = (char *)argv[1];
    char *dst = (char *)argv[2];
    if (mount(src, dst) < 0)
    {
        perror("mount");
        return 1;
    }

    return 0;
}
