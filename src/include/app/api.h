#pragma once

// api

int fork();

int execve(char *path, char *const argv[], char *const envp[]);

#define O_ACCMODE 0003 // 访问模式掩码
#define O_RDONLY  00   // 只读
#define O_WRONLY  01   // 只写
#define O_RDWR    02   // 读写

#define O_CREAT  0400  // 创建
#define O_EXCL   02000 // 互斥创建
#define O_TRUNC  01000 // 截断
#define O_APPEND 0010  // 末尾追加

int open(char *path, int flgs);

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

ssize_t write(int fd, const void *buf, size_t cnt);

ssize_t read(int fd, void *buf, size_t cnt);

int close(int fd);

