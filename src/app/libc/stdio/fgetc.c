#include "stdio.h"

/*
 * fgetc for stdin is not supported correctly yet. BS may be included in it!!!
 * when tty has been implemented, it could work well.
 */
int fgetc(FILE *f)
{
    if (f->rpos != f->rend)
        return *f->rpos++;

    unsigned char s;
    if (__toread(f))
        return EOF;
    if (f->read(f, &s, 1) != 1)
        return EOF;
    return s;
}

__alias(fgetc, getc);