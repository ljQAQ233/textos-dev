#include "stdio.h"
#include <errno.h>

int fileno(FILE *f)
{
    int fd = f->fd;
    if (fd < 0) {
        errno = EBADF;
        return -1;
    }
    return fd;
}
