#include "stdio.h"

/**
 * @brief flush a stream
 *   For output streams: flush user-space buffer to file.
 *   For seekable input streams: discard unread input buffer.
 *   Stream stays open. If stream is NULL, flush all output streams.
 */
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
