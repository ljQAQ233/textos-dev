#ifndef	_FCNTL_H
#define	_FCNTL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_mode_t

#include <bits/alltypes.h>

#define O_ACCMODE 0003 // 访问模式掩码
#define O_RDONLY  00   // 只读
#define O_WRONLY  01   // 只写
#define O_RDWR    02   // 读写

#define O_CREAT  0400  // 创建
#define O_EXCL   02000 // 互斥创建
#define O_TRUNC  01000 // 截断
#define O_APPEND 0010  // 末尾追加
#define O_DIRECTORY 0200000

int open(char *__path, int __flgs, ...);

#include <bits/perm.h>

__END_DECLS

#endif