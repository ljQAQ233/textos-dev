#include "stdio.h"
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>

FILE *__fdopen(int fd, int flgs)
{
    FILE *f = malloc(sizeof(FILE));
    if (!f) return 0;

    int fl = 0;
    switch (flgs & O_ACCMODE) {
    case O_RDONLY:
        fl |= F_NOWR;
        break;
    case O_WRONLY:
        fl |= F_NORD;
        break;
    default:
        break;
    }
    if (flgs & O_APPEND) fl |= F_APP;

    void *area = malloc(BUFSIZ + MAX_UNGETC);
    if (!area) return 0;
    f->fd = fd;
    f->fl = fl | F_ALOC;
    f->lbf = '\n';
    f->bufsz = BUFSIZ;
    f->buf = area + MAX_UNGETC;
    f->seek = __stdio_seek;
    f->read = __stdio_read;
    f->write = __stdio_write;
    f->close = __stdio_close;
    if (!f->bufsz) {
        free(f);
        return 0;
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
    if (flgs < 0) goto inv;
    int accm = flgs & O_ACCMODE;
    int orig = fcntl(fd, F_GETFL) & O_ACCMODE;
    if (orig != accm) goto inv;
    return __fdopen(fd, flgs);

inv:
    errno = EINVAL;
    return NULL;
}
