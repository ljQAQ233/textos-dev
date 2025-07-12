#include "stdio.h"
#include <unistd.h>
#include <sys/uio.h>

size_t __stdio_read(FILE *f, unsigned char *buf, size_t len)
{
    struct iovec iov[2] = {
        { .iov_base = buf, .iov_len = len },
        { .iov_base = f->buf, .iov_len = f->bufsz }
    };
    ssize_t cnt;
    cnt = iov[0].iov_len ? readv(f->fd, iov, 2)
        : read(f->fd, iov[1].iov_base, iov[1].iov_len);
    if (cnt <= 0) {
        f->fl |= cnt < 0 ? F_ERR : F_EOF;
        return 0;
    }
    if (cnt <= iov[0].iov_len)
        return cnt;
    cnt -= iov[0].iov_len;
    f->rpos = f->buf;
    f->rend = f->buf + cnt;
    return len;
}

size_t __stdio_write(FILE *f, const unsigned char *buf, size_t len)
{
    struct iovec iovs[2] = {
        { .iov_base = f->wbase, .iov_len = f->wpos - f->wbase },
        { .iov_base = (void *)buf, .iov_len = len }
    };
    struct iovec *iov = iovs;
    size_t rem = iov[0].iov_len + iov[1].iov_len;
    int iovcnt = 2;
    ssize_t cnt;
    for (;;) {
        cnt = writev(f->fd, iov, iovcnt);
        if (cnt == rem) {
            // 重置, 准备下一次写入
            f->wend = f->buf + f->bufsz;
            f->wpos = f->wbase = f->buf;
            return len;
        }
        if (cnt < 0) {
            f->wpos = f->wbase = f->wend = 0;
            f->fl |= F_ERR;
            if (iovcnt == 2)
                return 0;
            return len - iov[0].iov_len;
        }
        rem -= cnt;
        if (cnt > iov[0].iov_len) {
            cnt -= iov[0].iov_len;
            iov++; iovcnt--;
        }
        iov[0].iov_base = iov[0].iov_base + cnt;
        iov[0].iov_len -= cnt;
    }
}

int __stdio_close(FILE *f)
{
    close(f->fd);
    return 0;
}