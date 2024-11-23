#ifndef __FILE_H__
#define __FILE_H__

#include <textos/fs.h>
#include <textos/klib/list.h>

typedef struct {
    size_t offset;
    node_t *node;
    int flgs;
    int refer;
} file_t;

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

ssize_t write(int fd, void *buf, size_t cnt);

ssize_t read(int fd, void *buf, size_t cnt);

int close(int fd);

int dup(int fd);

int dup2(int old, int new);

#endif
