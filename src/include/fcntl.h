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

int open(const char *__path, int __flgs, ...);

#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2

struct flock
{
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};

#define F_DUPFD  0
#define F_GETFD  1
#define F_SETFD  2
#define F_GETFL  3
#define F_SETFL  4

#define F_GETLK  5
#define F_SETLK  6
#define F_SETLKW 7

#define F_SETOWN 8
#define F_GETOWN 9
#define F_SETSIG 10
#define F_GETSIG 11

#define FD_CLOEXEC 1

int fcntl(int __fd, int __cmd, ...);

#include <bits/perm.h>

__END_DECLS

#endif