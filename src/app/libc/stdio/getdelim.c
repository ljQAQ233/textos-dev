#include "stdio.h"
#include <string.h>
#include <malloc.h>

static void *__realloc(void *ptr, size_t nsiz, size_t osiz)
{
    void *np = malloc(nsiz);
    if (!np)
        return NULL;
    size_t sz = nsiz > osiz ? osiz : nsiz;
    memcpy(np, ptr, sz);
    free(ptr);
    return np;
}

/**
 * @brief 读取直到分割符 (含分隔符), 到 n 是传入的 *lineptr 的大小, 如果不足会扩增.
 * @note  无论函数成功与否, *lineptr 都必须由调用者释放.
 * 
 * @param lineptr  指向缓冲区的指针, 指向缓冲区, 或者 null 则自动扩增
 * @param n        缓冲区大小. 如果 原 *lineptr 不够用, n 会被设置为新的缓冲区大小
 * @param f        文件流
 * @return ssize_t >  0 读取长度 (包含分割符)
 *                 = -1 错误 / 文件结束
 */
// TODO: 识别已有的缓冲区 lineptr, 但是没有 malloc...
ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delim, FILE *restrict f)
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
            return EOF;
        if (f->read(f, 0, 0) != 0)
            return EOF;
        if (f->rpos == f->rend)
            return EOF;
    }
    ssize_t cnt = 0;
    char *buf = 0;
    unsigned char c = delim;
    unsigned char *dpos = 0;
    for (;;)
    {
        dpos = memchr(f->rpos, c, f->rend - f->rpos);
        size_t cpysz = (dpos ? dpos + 1 : f->rend) - f->rpos;
        char *tmp = __realloc(buf, cnt + cpysz + 1, cnt);
        if (!tmp)
            goto err;
        buf = tmp;
        memcpy(buf + cnt, f->rpos, cpysz);
        cnt += cpysz;
        f->rpos += cpysz;
        if (dpos)
            break;
        if (f->read(f, 0, 0) != 0)
            goto err;
        if (f->rpos == f->rend)
            break;
    }

err:
    if (!oldsz)
    {
        f->buf = oldbuf;
        f->bufsz = oldsz;
    }
    buf[cnt] = 0;
    *lineptr = buf;
    *n = cnt;
    return cnt ? cnt : EOF;
}