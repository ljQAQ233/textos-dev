#include "stdio.h"

int fputc(int c, FILE *f)
{
    unsigned char s = c;
    if (!f->wpos && __towrite(f))
        return EOF;
    if (f->wpos != f->wend && c != f->lbf)
        return *f->wpos++ = c;
    if (f->write(f, &s, 1) != 1)
        return EOF;
    return c;
}

__alias(fputc, putc);