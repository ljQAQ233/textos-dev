#ifndef _STRING_H
#define _STRING_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_size_t
#include <bits/alltypes.h>
#include <bits/null.h>

/*
  The `strlen()` function calculates the 
  length of the string pointed to by `str`,
  excluding the terminating null byte ('\0')
*/
size_t strlen(const char *__str);

size_t strnlen(const char *str, size_t maxlen);

/*
  The `strcpy()` function copies the string
  pointed to by `src`, including the terminating
  null byte ('\0')
*/
char *strcpy(char *__dest, const char *__src);

/* Limited by `n`, is similar to `strcpy()` */
char *strncpy(char *__dest, const char *__src, size_t __n);

/* The `strcmp()` function compares the two strings `str1` and `str2` */
int strcmp(const char* __str1,const char *__str2);

/*
  The `strncmp()` function is similar to `strcmp()`, except it
  compares only the first (at most) n bytes of `str1` and `str2`
*/
int strncmp(const char* __str1, const char *__str2, size_t __n);

char *strcat(char* __dest, const char* __src);

/*
  The `strchr()` function returns a pointer to the first
  occurrence of the character `c` in the string `str`
*/
char *strchr(const char *__str, int __c);
char *strrchr(const char *__str, int __c);
char *strchrnul(const char *__str, int __c);

#ifdef _GNU_SOURCE
char *strrchrnul(const char *__str, int __c);
#endif

char *strstr(const char *__haystack, const char *__needle);

/*
  The `strspn()` function calculates the length (in bytes) of the initial
  segment of `s` which consists entirely of bytes in `accept`
*/
size_t strspn(const char *__str, const char *__accept);

/*
  string complement span
 */
size_t strcspn(const char *__str, const char *__reject);

/*
  The `strpbrk()` function locates the first occurrence in the string `s`
  of any of the bytes in the string `accept`
 */
char *strpbrk(const char *__str, const char *__accept);

/* duplicate a string */
char *strdup(const char *__str);

/* duplicate a string, limited by `n` */
char *strndup(const char *__str, size_t __n);

/* 
  The `memset()` function  fills  the first `n` bytes of the
  memory area pointed to by `str` with the constant byte `c`
*/
void *memset(void *__dest, int __c, size_t __n);

/* Compare memory areas (`n` bytes) */
int memcmp(const void *__ptr1, const void *__ptr2, size_t __n);

/*
  returns a pointer to the first occurrence of the uchar `c`
  in the space start from `ptr`. If the pointer finder holds
  run out of range, return NULL
*/
void *memchr(const void *__ptr, int __c, size_t __n);

/*
  The `memcpy()` function  copies `n`  bytes
  from memory area `src` to memory area `dest`
*/
void *memcpy(void *__dest, const void *__src, size_t __n);

char *strerror(int __errnum);

// TODO : Other functions in string.h

__END_DECLS

#endif
