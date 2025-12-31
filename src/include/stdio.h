#ifndef _STDIO_H
#define _STDIO_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdarg.h>
#include <stddef.h>
#include <bits/null.h>

#define EOF ((int)-1)
#define BUFSIZ 4096

#define __NEED_off_t
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

ssize_t getline(char **restrict __lineptr, size_t *restrict __n, FILE *restrict __f);
ssize_t getdelim(char **restrict __lineptr, size_t *restrict __n, int __delim, FILE *restrict __f);

int printf(const char *__format, ...);
int fprintf(FILE *f, const char *format, ...);
int dprintf(int __fd, const char *__format, ...);
int sprintf(char *__buf, const char *__format, ...);
int snprintf(char *__buf, size_t __n, const char *__format, ...);
int vprintf(const char *__format, va_list __ap);
int vfprintf(FILE *__f, const char *__format, va_list __ap);
int vdprintf(int __fd, const char *__format, va_list __ap);
int vsprintf(char *__buf, const char *__format, va_list __ap);
int vsnprintf(char *__buf, size_t n, const char *__format, va_list __ap);
void perror(const char *__s);

int fileno(FILE *__f);
FILE *fopen(const char *__path, const char *__mode);
FILE *fdopen(int __fd, const char *__mode);
int fflush(FILE *__f);
int fclose(FILE *__f);
void clearerr(FILE *__f);
int feof(FILE *__f);
int ferror(FILE *__f);
off_t ftello(FILE *f);
long ftell(FILE *f);

int fgetc(FILE *__f);
int fputc(int __c, FILE *__f);

size_t fread(void *restrict __ptr, size_t __size, size_t __nmemb, FILE *restrict __f);
size_t fwrite(const void *restrict __ptr, size_t __size, size_t __nmemb, FILE *restrict __f);

int fputs(const char *restrict __s, FILE *restrict __f);
char *fgets(char *restrict __s, int __n, FILE *restrict __f);

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

int setvbuf(FILE *__f, char *__buf, int __mode, size_t __size);
void setbuf(FILE *__f, char *__buf);

__END_DECLS

#endif
