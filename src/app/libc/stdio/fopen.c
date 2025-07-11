#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include "stdio.h"

FILE *fopen(const char *path, const char *mode)
{
    int fd;
    int flgs;
    FILE *f;

    flgs = __fmode(mode);
    if (flgs < 0)
    {
        errno = -flgs;
        return NULL;
    }
    fd = open(path, flgs, 0666);
    if (fd < 0)
        return NULL;

    f = __fdopen(fd);
    if (!f)
        close(fd);
    return f;
}