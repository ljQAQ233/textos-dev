#ifndef _STDIO_H
#define _STDIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdarg.h>
#include <stddef.h>
#include <bits/null.h>

#define EOF ((int)-1)
#define BUFSIZ 4096

#define __NEED_ssize_t
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
int getc(FILE *__f);
int getchar();

ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *f);
ssize_t getdelim(char **restrict __lineptr, size_t *restrict __n, int __delim, FILE *restrict __f);

int printf(char *__format, ...);
int sprintf(char *__buffer, const char *__format, ...);
int dprintf(int __fd, char *__format, ...);
int vsprintf(char *__buffer, const char *__format, va_list __args);
void perror(const char *__s);

FILE *fopen(const char *__path, const char *__mode);
FILE *fdopen(int __fd, const char *__mode);
int fflush(FILE *__f);
int fclose(FILE *__f);

int fgetc(FILE *__f);
int fputc(int __c, FILE *__f);

size_t fread(void *restrict __ptr, size_t __size, size_t __nmemb, FILE *restrict __f);
size_t fwrite(const void *restrict __ptr, size_t __size, size_t __nmemb, FILE *restrict __f);

int fputs(const char *restrict __s, FILE *restrict __f);
char *fgets(char *restrict __s, int __n, FILE *restrict __f);

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

int setvbuf(FILE *__f, char *__buffer, int __mode, size_t __size);

__END_DECLS

#endif