#ifndef _STDIO_H
#define _STDIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdarg.h>
#include <bits/null.h>

int putchar(int __c);

int puts(char *__s);

int printf(char *__format, ...);

int sprintf(char *__buffer, const char *__format, ...);

int dprintf(int __fd, char *__format, ...);

int vsprintf(char *__buffer, const char *__format, va_list __args);

void perror(const char *__s);

__END_DECLS

#endif