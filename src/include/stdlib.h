#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_int8_t
#define __NEED_int32_t
#define __NEED_size_t
#include <bits/null.h>
#include <bits/alltypes.h>

int abs(int __x);
long labs(long __x);
long long llabs(long long __x);

typedef struct { int quot, rem; } div_t;
typedef struct { long quot, rem; } ldiv_t;
typedef struct { long long quot, rem; } lldiv_t;

div_t div(int __num, int __den);
ldiv_t ldiv(long __num, long __den);
lldiv_t lldiv(long long __num, long long __den);

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
void _Exit(int __status);
void exit(int __status);

// environ
int setenv(const char *__name, const char *__value, int __overwrite);
int putenv(char *__str);
int unsetenv(const char *__name);
char *getenv(const char *__name);

// number / string
long strtol(const char *restrict __nptr,
            char /* _Nullable */ **restrict __endptr, int __base);
long long strtoll(const char *restrict nptr,
                  char /* _Nullable */ **restrict __endptr, int __base);
unsigned long strtoul(const char *restrict nptr,
                      char /* _Nullable */ **restrict __endptr, int __base);
unsigned long long strtoull(const char *restrict nptr,
                            char /* _Nullable */ **restrict __endptr,
                            int __base);
int atoi(const char *__str);
long atol(const char *__str);
long long atoll(const char *__str);

_Noreturn void abort();

// random

/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables.  */
struct random_data
{
    int32_t *fptr;  /* Front pointer.  */
    int32_t *rptr;  /* Rear pointer.  */
    int32_t *state; /* Array of state values.  */
#if 0
    int rand_type; /* Type of random number generator.  */
    int rand_deg;  /* Degree of random number generator.  */
    int rand_sep;  /* Distance between front and rear.  */
#else
    /* random_r.c, TYPE_x, DEG_x, SEP_x - small enough for int8_t */
    int8_t rand_type; /* Type of random number generator.  */
    int8_t rand_deg;  /* Degree of random number generator.  */
    int8_t rand_sep;  /* Distance between front and rear.  */
#endif
    int32_t *end_ptr; /* Pointer behind state table.  */
};

// INT_MAX
#define RAND_MAX 2147483647

// Reentrant version
int random_r(struct random_data *__buf, int32_t *__result);
int srandom_r(unsigned int __seed, struct random_data *__buf);
int initstate_r(unsigned int __seed, char *__arg_state, size_t __n,
                struct random_data *__buf);
int setstate_r(char *__arg_state, struct random_data *__buf);

// Global state used
void srandom(unsigned int __x);
char *initstate(unsigned int __seed, char *__arg_state, size_t __n);
char *setstate(char *__arg_state);
long int random();

// POSIX.1-2001. Obsolete in POSIX.1-2008.
int rand_r(unsigned *__seed);

__END_DECLS

#endif
