#ifndef __FILE_H__
#define __FILE_H__

#include <textos/fs.h>

typedef struct {
    bool occupied;
    size_t offset;
    node_t *node;
} file_t;

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define MAXDEF_FILENO 3

extern file_t sysfile[MAXDEF_FILENO];

ssize_t write(int fd, void *buf, size_t cnt);

ssize_t read(int fd, void *buf, size_t cnt);

int close(int fd);

#endif