#include "stdio.h"
#include <malloc.h>

int __updbuf(FILE *f, void *buf, size_t size, int alloc)
{
    void *old_alloc = 0;
    if (f->fl & F_ALOC)
        old_alloc = f->buf;
    if (!buf && alloc)
    {
        if (!size)
            size = BUFSIZ;
        buf = malloc(size);
        if (!buf)
            return -1;
        f->fl |= F_ALOC;
    }
    else
    {
        f->fl &= ~F_ALOC;
    }
    f->buf = buf;
    f->bufsz = size;
    if (old_alloc)
        free(old_alloc);
    return 0;
}

int setvbuf(FILE *f, char *buf, int mode, size_t size)
{
    if (mode == _IOFBF || mode == _IOLBF)
    {
        if (__updbuf(f, buf, size, 1) < 0)
            return -1;
        f->lbf = mode == _IOFBF ? EOF : '\n';
    }
    else if (mode == _IONBF)
    {
        if (__updbuf(f, 0, 0, 0) < 0)
            return -1;
        f->lbf = EOF;
    }
    else
    {
        return -1;
    }
    f->rpos = f->rend = 0;
    f->wbase = f->wpos = f->wend = 0;
    return 0;
}
