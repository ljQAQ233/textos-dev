#include <app/api.h>
#include <stdio.h>
#include <string.h>

void tree(char *path, int depth)
{
    stat_t sb;
    if (stat(path, &sb) < 0)
        return;

    if (!S_ISDIR(sb.mode))
        return;

    int fd = open(path, O_DIRECTORY);
    if (fd < 0)
        return;

    int nr;
    char b[1024];
    while ((nr = __readdir(fd, b, sizeof(b))) > 0)
    {
        dir_t *p = (dir_t *)b;
        while (p < (dir_t *)(b + nr)) {
            if (p->name[0] != '.') {
                for (int i = 0; i < depth; i++)
                    printf("  ");
                printf("|- %s\n", p->name);

                char sub[256];
                int len = strlen(path);
                strcpy(sub, path);
                if (len && path[len - 1] != '/')
                    strcat(sub, "/");
                strcat(sub, p->name);

                stat_t st;
                if (stat(sub, &st) == 0 && S_ISDIR(st.mode))
                    tree(sub, depth + 1);
            }
            p = (void *)p + p->siz;
        }
    }

    close(fd);
}

int main(int argc, char const *argv[])
{
    char *start = argc > 1 ? (char *)argv[1] : ".";
    printf("%s\n", start);
    tree(start, 0);
    return 0;
}
