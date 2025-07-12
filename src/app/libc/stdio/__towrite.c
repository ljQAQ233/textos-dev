#include "stdio.h"

// 准备写入
int __towrite(FILE *f)
{
	if (f->fl & F_NOWR)
    {
		f->fl |= F_ERR;
		return EOF;
	}
	// 撤销 读缓冲
	f->rpos = f->rend = 0;

	// 设置 写缓冲
	f->wpos = f->wbase = f->buf;
	f->wend = f->buf + f->bufsz;

	return 0;
}