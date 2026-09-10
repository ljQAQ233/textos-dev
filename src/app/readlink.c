#include <malloc.h>
#include <unistd.h>

#include "util.h"
PROG_NAME("readlink");

#define DEFBUFSZ 8192
#define SETP     1024

int rdlink(const char *path, char **result)
{
    int ret = 0;
    size_t bufsz = DEFBUFSZ;
    char *buf = NULL;
    for (int retry = 0; retry < 4; retry++) {
        buf = realloc(buf, bufsz);
        if (!buf) break;
        ret = readlink(path, buf, bufsz - 1);
        if (ret != -1) break;
        bufsz += SETP;
    }
    if (buf) *result = buf;
    return ret;
}

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        pusage("file ...");
        return 1;
    }
    int err = 0;
    char *res;
    for (int i = 1 ; i < argc ; i++) {
        int ret = rdlink(argv[i], &res);
        if (ret < 0)
            ;
        else
            puts(res);
        free(res);
        err |= ret < 0;
    }
    return !!err;
}
