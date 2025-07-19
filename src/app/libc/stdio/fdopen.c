#include "stdio.h"
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>

FILE *__fdopen(int fd, int flgs)
{
    FILE *f = malloc(sizeof(FILE));
    if (!f)
        return NULL;

    int fl = 0;
    switch (flgs & O_ACCMODE)
    {
    case O_RDONLY: fl |= F_NOWR; break;
    case O_WRONLY: fl |= F_NORD; break;
    default: break;
    }
    if (flgs & O_APPEND)
        fl |= F_APP;
    
    f->fd = fd;
    f->fl = fl;
    f->lbf = '\n';
    f->bufsz = BUFSIZ;
    f->buf = malloc(BUFSIZ);
    f->read = __stdio_read;
    f->write = __stdio_write;
    f->close = __stdio_close;
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
    return __fdopen(fd, flgs);
    
inv:
    errno = EINVAL;
    return NULL;
}
