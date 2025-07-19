#include "stdio.h"
#include <unistd.h>

/*
 * 考虑 O_APPEND 的情况: lseek 返回的 pos 不一定是下一次写入的位置,
 * 有可能 write 之前 lseek 调整过位置, 尽管下一次写入时这个新的值会被忽略.
 * 在 linux 上: write 一个 O_APPEND 的文件会将 offset 自动拨到文件末尾.
 */
off_t __fdpos(FILE *f)
{
    if ((f->fl & F_APP) && f->wbase != f->wpos)
        return f->seek(f, 0, SEEK_END);
    return f->seek(f, 0, SEEK_CUR);
}

/*
 * _FILE_OFFSET_BITS == 64 || _POSIX_C_SOURCE >= 200112L
 */
off_t ftello(FILE *f)
{
    off_t pos = __fdpos(f);
    if (pos < 0)
        return pos;

    if (f->rend)
        pos += f->rpos - f->rend;
    else if (f->wbase)
        pos += f->wpos - f->wbase;
    return pos;
}
