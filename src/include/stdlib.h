#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <bits/null.h>

int abs(int __x);
long labs(long __x);
long long llabs(long long __x);

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

div_t div(int __num, int __den);
ldiv_t ldiv(long __num, long __den);
lldiv_t lldiv(long long __num, long long __den);

void _Exit(int __status);

void exit(int __status);

// environ
int setenv(const char *__name, const char *__value, int __overwrite);
int putenv(char *__str);
int unsetenv(const char *__name);
char *getenv(const char *__name);

__END_DECLS

#endif