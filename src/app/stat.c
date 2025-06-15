#include <stdio.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

void dump(char *path, struct stat *sb)
{
    printf("  File: %s\n", path);
    printf("  Size: %lu\n", sb->st_size);
    printf("Device: %u, %u\n", major(sb->st_dev), minor(sb->st_dev));
}

int main(int argc, char const *argv[])
{
    if (argc == 1)
        return 1;

    struct stat sb;
    for (int i = 1 ; i < argc ; i++)
    {
        char *path = (char *)argv[i];
        if (stat(path, &sb) < 0)
            perror("stat");
        else
            dump(path, &sb);
    }

    return 0;
}
