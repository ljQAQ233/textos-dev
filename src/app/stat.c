#include <app/api.h>
#include <stdio.h>

void dump(char *path, stat_t *sb)
{
    printf("  File: %s\n", path);
    printf("  Size: %lu\n", sb->siz);
    printf("Device: %u, %u\n", major(sb->dev), minor(sb->dev));
}

int main(int argc, char const *argv[])
{
    if (argc == 1)
        return 1;

    stat_t sb;
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
