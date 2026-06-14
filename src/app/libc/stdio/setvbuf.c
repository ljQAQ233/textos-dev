#include "stdio.h"
#include <malloc.h>

int __updbuf(FILE *f, void *area, size_t size, int alloc)
{
    // POSIX: the array it points to **may** be used instead of a buffer
    // allocated by setvbuf(). 这个 "may" 就很灵性, 也就是允许标准拒绝设置用户
    // buffer, musl 用这个特性来拒绝不符合 ungetc 要求的 buffer... 这里称 buf 为
    // area 是因为实现起来 area 包含了 buf 与 ungetc_buf
    if (size < MAX_UNGETC) return 0;
    void *old_alloc = 0;
    if (f->fl & F_ALOC) old_alloc = f->buf;
    if (!area && alloc) {
        if (!size) size = BUFSIZ;
        area = malloc(size);
        if (!area) return -1;
        f->fl |= F_ALOC;
    } else {
        f->fl &= ~F_ALOC;
    }
    f->buf = area + MAX_UNGETC;
    f->bufsz = size - MAX_UNGETC;
    if (old_alloc) free(old_alloc);
    return 0;
}

int setvbuf(FILE *f, char *buf, int mode, size_t size)
{
    if (mode == _IOFBF || mode == _IOLBF) {
        if (__updbuf(f, buf, size, 1) < 0) return -1;
        f->lbf = mode == _IOFBF ? EOF : '\n';
    } else if (mode == _IONBF) {
        if (__updbuf(f, 0, 0, 0) < 0) return -1;
        f->lbf = EOF;
    } else {
        return -1;
    }
    f->rpos = f->rend = 0;
    f->wbase = f->wpos = f->wend = 0;
    return 0;
}
