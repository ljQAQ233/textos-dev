#include "stdio.h"
#include <string.h>

size_t __fwritex(const unsigned char *restrict buf, size_t len, FILE *restrict f)
{
    size_t i = 0;
    if (!f->wend && __towrite(f))
        return 0;

    // 缓冲区满, 输出 buf + 缓冲区
    if (len > f->wend - f->wpos)
        return f->write(f, buf, len);
    // 检查换行符, 输出
    if (f->lbf >= 0)
    {
        for (i = len; i && buf[i-1] != '\n'; i--);
        if (i) {
            // 输出换行符前的一段
            // n < i 说明不能再写了, 返回
            size_t n = f->write(f, buf, i);
            if (n < i)
                return n;
            buf += i;
            len -= i;
        }
    }

    memcpy(f->wpos, buf, len);
    f->wpos += len;
    return len + i;
}

size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict f)
{
    if (!size || !nmemb)
        return 0;
    size_t l = size * nmemb;
    size_t i = __fwritex(ptr, l, f);
    return i == l ? nmemb : i / size;
}