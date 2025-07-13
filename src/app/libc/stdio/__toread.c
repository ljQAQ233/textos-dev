#include "stdio.h"

int __toread(FILE *f)
{
    if (f->wbase != f->wpos)
        f->write(f, NULL, 0);
    f->wbase = f->wpos = f->wend = 0;
    if (f->fl & F_NORD)
    {
        f->fl |= F_ERR;
        return EOF;
    }
    f->rpos = f->rend = f->buf + f->bufsz;
    return f->fl & F_EOF ? EOF : 0;
}