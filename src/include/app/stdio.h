#ifndef _STDIO_H
#define _STDIO_H 1

#include <stdarg.h>

int putchar(int c);

int puts(char *s);

int printf(char *format, ...);

int sprintf(char *buffer, const char *format, ...);

int dprintf(int fd, char *format, ...);

int vsprintf(char *buffer, const char *format, va_list args);

void perror(const char *s);

#endif
