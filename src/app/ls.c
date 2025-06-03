#include <app/api.h>

#include <stdio.h>

char b[4096];

int ls(char *path)
{
    stat_t sb;
    if (stat(path, &sb) < 0) {
        perror("ls");
        return 1;
    }

    if (S_ISDIR(sb.mode)) {
        int fd = open(path, O_DIRECTORY);
        if (fd < 0) {
            perror("ls");
            return 1;
        }

        int nr;
        while ((nr = __readdir(fd, b, sizeof(b))) > 0)
        {
            dir_t *p = (dir_t *)b;
            while (p < (dir_t *)(b + nr)) {
                if (p->name[0] != '.')
                    printf("%s\n", p->name);
                p = (void *)p + p->siz;
            }
        }
    } else {
        printf("%s\n", path);
    }

    return 0;
}

int main(int argc, char const *argv[])
{
    int ret = 0;
    if (argc == 1) {
        ret |= ls(".");
    } else {
        for (int i = 1 ; i < argc ; i++)
            ret |= ls((char  *)argv[i]);
    }
    return ret;
}
