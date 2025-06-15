#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>

void tree(char *path, int depth)
{
    struct stat sb;
    if (stat(path, &sb) < 0)
        return;

    if (!S_ISDIR(sb.st_mode))
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

                struct stat st;
                if (stat(sub, &st) == 0 && S_ISDIR(st.st_mode))
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
