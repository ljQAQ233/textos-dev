#ifndef _STDIO_H
#define _STDIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdarg.h>
#include <stddef.h>
#include <bits/null.h>

#define EOF ((int)-1)
#define BUFSIZ 4096

typedef struct
{
    int _f_fd;
    size_t _f_bufsz;
    void *_f_buf;
    void *_f_next;
} FILE;

int putchar(int __c);

int puts(char *__s);

int printf(char *__format, ...);

int sprintf(char *__buffer, const char *__format, ...);

int dprintf(int __fd, char *__format, ...);

int vsprintf(char *__buffer, const char *__format, va_list __args);

void perror(const char *__s);

FILE *fopen(const char *__path, const char *__mode);
FILE *fdopen(int __fd, const char *__mode);
int fclose(FILE *__stream);

__END_DECLS

#endif