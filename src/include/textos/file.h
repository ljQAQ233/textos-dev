#ifndef __FILE_H__
#define __FILE_H__

#include <textos/fs.h>
#include <textos/klib/list.h>

#define S_PIPE_R 0x01  // rx
#define S_PIPE_W 0x02  // tx

typedef struct {
    size_t offset;
    node_t *node;
    int flgs;
    int spec;
    int refer;
} file_t;

typedef struct dirent
{
    int idx;
    size_t siz;
    size_t len;
    char name[];
} dir_t;

#define S_IFMT 0170000

#define S_IFDIR  0040000
#define S_IFCHR  0020000
#define S_IFBLK  0060000
#define S_IFREG  0100000
#define S_IFIFO  0010000
#define S_IFLNK  0120000
#define S_IFSOCK 0140000

#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFMT) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)

typedef struct stat
{
    int mode;
    long dev;
    size_t siz;
} stat_t;

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAXDEF_FILENO 3

extern file_t sysfile[MAXDEF_FILENO];

int open(char *path, int flgs);

#define O_ACCMODE 0003 // 访问模式掩码
#define O_RDONLY  00   // 只读
#define O_WRONLY  01   // 只写
#define O_RDWR    02   // 读写

#define O_CREAT  0400  // 创建
#define O_EXCL   02000 // 互斥创建
#define O_TRUNC  01000 // 截断
#define O_APPEND 0010  // 末尾追加
#define O_DIRECTORY 0200000

ssize_t write(int fd, void *buf, size_t cnt);

ssize_t readdir(int fd, void *buf, size_t mx);

ssize_t read(int fd, void *buf, size_t cnt);

int close(int fd);

int stat(char *path, stat_t *sb);

int dup(int fd);

int dup2(int old, int new);

int pipe(int fds[2]);

int mknod(char *path, int mode, long dev);

#endif
