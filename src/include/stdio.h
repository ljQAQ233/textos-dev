#ifndef _STDIO_H
#define _STDIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdarg.h>
#include <stddef.h>
#include <bits/null.h>

#define EOF ((int)-1)
#define BUFSIZ 4096

#define __NEED_struct__IO_FILE
#define __NEED_FILE
#include <bits/alltypes.h>

extern FILE __stdio_stdin;
extern FILE __stdio_stdout;
extern FILE __stdio_stderr;

#define stdin  (&__stdio_stdin)
#define stdout (&__stdio_stdout)
#define stderr (&__stdio_stderr)

int putc(int __c, FILE *__f);
int putchar(int __c);

int puts(const char *__s);

int printf(char *__format, ...);

int sprintf(char *__buffer, const char *__format, ...);

int dprintf(int __fd, char *__format, ...);

int vsprintf(char *__buffer, const char *__format, va_list __args);

void perror(const char *__s);

FILE *fopen(const char *__path, const char *__mode);

FILE *fdopen(int __fd, const char *__mode);

int fclose(FILE *__f);

int fputc(int __c, FILE *__f);
size_t fwrite(const void *restrict __buf, size_t __size, size_t __nmemb, FILE *restrict __f);

/**
 * @brief writes the string s to stream, without its terminating null byte ('\0').
 * 
 * @return int nonnegative value if scuuess, or EOF on error
 */
int fputs(const char *restrict __s, FILE *restrict __f);

/**
 * @brief flush a stream
 *   For output streams: flush user-space buffer to file.
 *   For seekable input streams: discard unread input buffer.
 *   Stream stays open. If stream is NULL, flush all output streams.
 */
int fflush(FILE *__f);

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

int setvbuf(FILE *__f, char *__buffer, int __mode, size_t __size);

__END_DECLS

#endif