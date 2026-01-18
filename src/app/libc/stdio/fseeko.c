#include "stdio.h"
#include <errno.h>

int fseeko(FILE *f, off_t offset, int whence)
{
    if (whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
        errno = EINVAL;
        return -1;
    }
    // 用户看到的是 stdio 封装的 offset (见 ftell) 所以需要转化成底层的偏移量
    // 校正 SEEK_CUR 的偏移量: 考虑 buf 中未读的数据
    if (whence == SEEK_CUR && f->rend)
        offset -= f->rend - f->rpos;
    // 回写当前内容
    if (f->wpos != f->wbase)
    {
        f->write(f, 0, 0);
        if (f->wpos == 0)
            return -1;
        f->wpos = f->wbase = f->wend = 0;
    }
    if (f->seek(f, offset, whence) < 0)
        return -1;
    f->rpos = f->rend = 0;
    f->fl &= ~F_EOF;
    return 0;
}
