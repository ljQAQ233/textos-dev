#include "stdio.h"
#include <string.h>

#define EOL '\n'

char *fgets(char *restrict s, int n, FILE *restrict f)
{
    char interbuf[512];
    unsigned char *oldbuf = 0;
    size_t oldsz = 0;
    if (!f->bufsz)
    {
        oldbuf = f->buf;
        oldsz = f->bufsz;
        f->buf = interbuf;
        f->bufsz = sizeof(interbuf);
    }
    if (f->rpos == f->rend)
    {
        if (__toread(f) < 0)
            return NULL;
        if (f->read(f, 0, 0) != 0)
            return NULL;
        if (f->rpos == f->rend)
            return NULL;
    }
    size_t rem = n - 1;
    ssize_t cnt = 0;
    unsigned char *dpos = 0;
    for (;;)
    {
        dpos = memchr(f->rpos, EOL, f->rend - f->rpos);
        size_t cpysz = (dpos ? dpos + 1 : f->rend) - f->rpos;
        if (cpysz > rem)
            cpysz = rem;
        memcpy(s + cnt, f->rpos, cpysz);
        rem -= cpysz;
        cnt += cpysz;
        f->rpos += cpysz;
        if (dpos || !rem)
            break;
        if (f->read(f, 0, 0) != 0)
            break;
        if (f->rpos == f->rend)
            break;
    }

    if (!oldsz)
    {
        f->buf = oldbuf;
        f->bufsz = oldsz;
    }
    s[n - rem - 1] = 0;
    return s;
}