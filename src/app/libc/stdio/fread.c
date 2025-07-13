#include "stdio.h"
#include <string.h>

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict f)
{
    size_t len = size * nmemb;
    size_t rem = len;

    // 处理缓冲区中的数据
    if (f->rpos != f->rend)
    {
        size_t bsz = f->rend - f->rpos;
        if (bsz > size)
            bsz = size;
        memcpy(ptr, f->rpos, bsz);
        ptr += bsz;
        rem -= bsz;
    }
    while (rem)
    {
        ssize_t ret;
        if (__toread(f))
            goto end;
        ret = f->read(f, ptr, rem);
        if (ret <= 0)
            goto end;
        rem -= ret;
        ptr += ret;
    }
end:
    return (len - rem) / size;
}
