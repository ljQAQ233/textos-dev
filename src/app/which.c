#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

char buf[4096];

int which(const char *name)
{
    size_t namelen = strlen(name);
    char *cur = getenv("PATH");
    char *nxt = cur;
    size_t len;
    struct stat sb;
    for (; nxt; cur = nxt + 1) {
        nxt = strchr(cur, ':');
        if (nxt)
            len = nxt - cur;
        else
            len = strlen(cur);
        if (len + namelen + 1 > 4096) {
            continue;
        }
        strncpy(buf, cur, len);
        buf[len] = '/';
        strncpy(buf + len + 1, name, namelen);
        buf[len + 1 + namelen] = '\0';

        // fprintf(stderr, "probe %s\n", buf);
        if (stat(buf, &sb) < 0)
            continue;
        if (S_ISREG(sb.st_mode))
            return 0;
    }
    return -1;
}

int main(int argc, char const *argv[])
{
    int i;
    for (i = 1; i < argc; i++) {
        if (which(argv[i]) < 0) {
            printf("%s not found\n", argv[i]);
        } else {
            puts(buf);
        }
    }
    return i == 1;
}
