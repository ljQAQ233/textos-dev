#include <app/api.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
    if (argc != 2)
        return 1;

    char *path = (char *)argv[1];
    if (mkdir(path, 0777) < 0)
    {
        perror("mkdir");
        return 1;
    }
    return 0;
}
