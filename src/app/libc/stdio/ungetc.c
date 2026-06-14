#include "stdio.h"

int ungetc(int c, FILE *f)
{
    if (c == EOF) return EOF;
    if (__toread(f)) return EOF;
    if (f->rpos <= (unsigned char *)f->buf - MAX_UNGETC) return EOF;
    *--f->rpos = c;
    f->fl &= ~F_EOF;
    return c;
}
