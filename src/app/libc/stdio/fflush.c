#include "stdio.h"

int fflush(FILE *f)
{
    if (f != NULL)
    {
        if (f->wpos != f->wbase && f->write(f, NULL, 0))
            return EOF;
        f->wpos = f->wend = f->wbase = 0;
        // TODO: f->rpos = f->rend = 0;
        return 0;
    }

    int ret = 0;
    FILE *p = __ofl_get();
    while (p)
    {
        ret |= fflush(p);
        p = p->next;
    }
    return ret;
}