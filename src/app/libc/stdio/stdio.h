#define __DEFINED_struct__IO_FILE

struct _IO_FILE;

#include <stdio.h>

#define F_PERM 1
#define F_NORD 2
#define F_NOWR 4
#define F_EOF  8
#define F_ERR  16

struct _IO_FILE
{
    int fd;
    int fl;
    int lbf;
    size_t pos; // TODO
    size_t bufsz;
    void *buf;
    size_t (*read)(FILE *f, unsigned char *buf, size_t len);
    size_t (*write)(FILE *f, const unsigned char *buf, size_t len);
    int (*close)(FILE *f);
    
    unsigned char *rpos;  // 下一次读取
    unsigned char *rend;  // 缓冲区末端
    unsigned char *wpos;  // 下一次写入
    unsigned char *wend;  // 缓冲区末端
    unsigned char *wbase; // 写缓冲基址
    FILE *next;
};

FILE *__fdopen(int fd, int flgs);
int __fmode(const char *mode);

void __ofl_add(FILE *f);
void __ofl_del(FILE *f);
FILE *__ofl_get();

int __toread(FILE *f);
int __towrite(FILE *f);

size_t __stdio_read(FILE *f, unsigned char *buf, size_t len);
size_t __stdio_write(FILE *f, const unsigned char *buf, size_t len);
int __stdio_close(FILE *f);

size_t __fwritex(const unsigned char *restrict buf, size_t len, FILE *restrict f);