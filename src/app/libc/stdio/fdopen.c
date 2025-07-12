#include "stdio.h"
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>

FILE *__fdopen(int fd)
{
    FILE *f = malloc(sizeof(FILE));
    if (!f)
        return NULL;
    
    f->fd = fd;
    f->bufsz = BUFSIZ;
    f->buf = malloc(BUFSIZ);
    if (!f->bufsz)
    {
        free(f);
        return NULL;
    }
    return f;
}

int __fmode(const char *mode);

/*
 * The mode of the stream (one of the values "r", ...) must be compatible with
 * the mode of the file descriptor.
 */
FILE *fdopen(int fd, const char *mode)
{
    int flgs = __fmode(mode);
    if (flgs < 0)
        goto inv;
    int accm = flgs & O_ACCMODE;
    int orig = fcntl(fd, F_GETFD);
    if (orig != accm)
        goto inv;
    return __fdopen(fd);
    
inv:
    errno = EINVAL;
    return NULL;
}