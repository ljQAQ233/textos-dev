#include "stdio.h"
#include <fcntl.h>
#include <unistd.h>

FILE *freopen(const char *path, const char *mode, FILE *f)
{
    fflush(f);
    // If path is a null pointer, freopen() changes the mode of
    // the stream to that specified in mode
    if (!path) {
        int nfl = __fmode(mode);
        if (fcntl(f->fd, F_SETFL, nfl) < 0) {
            fclose(f);
            return 0;
        }
        return f;
    }
    // 调用者不应该假设, freopen 也会处理底层文件描述符, 也就是:
    // freopen("f", "w", stdout) 之后不能假设 fd=1 也指向 "f"
    FILE *nf = fopen(path, mode);
    // 根据 POSIX, 无论 open 操作成功与否, 原流都应该被 "关闭"
    if (!nf) {
        fclose(f);
        return 0;
    }

    f->fl &= ~(F_ERR | F_EOF);
    f->fd = dup(nf->fd);
    f->fl = nf->fl;
    f->lbf = nf->lbf;
    f->pos = nf->pos;
    if (~f->fl & F_ALOC) {
        // use default buffer
        if (__updbuf(f, 0, 0, 1) < 0) {
            fclose(f);
            fclose(nf);
            return 0;
        }
    }
    f->cookie = 0;
    f->seek = nf->seek;
    f->read = nf->read;
    f->write = nf->write;
    f->close = nf->close;
    f->rpos = f->rend = 0;
    f->wpos = f->wend = f->wbase = 0;
    fclose(nf);
    return f;
}
