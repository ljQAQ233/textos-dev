#ifndef _STDIO_H
#define _STDIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdarg.h>
#include <bits/null.h>

int putchar(int c);

int puts(char *s);

int printf(char *format, ...);

int sprintf(char *buffer, const char *format, ...);

int dprintf(int fd, char *format, ...);

int vsprintf(char *buffer, const char *format, va_list args);

void perror(const char *s);

__END_DECLS

#endif