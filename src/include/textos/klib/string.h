#ifndef __STRING_H__
#define __STRING_H__

/*
  The `strlen()` function calculates the 
  length of the string pointed to by `str`,
  excluding the terminating null byte ('\0')
*/
size_t strlen (const char *str);

/*
  The `strcpy()` function copies the string
  pointed to by `src`, including the terminating
  null byte ('\0')
*/
char *strcpy (char *dest, const char *src);

/* Limited by `n`, is similar to `strcpy()` */
char *strncpy (char *dest, const char *src, size_t n);

/* The `strcmp()` function compares the two strings `str1` and `str2` */
int strcmp (const char* str1,const char *str2);

/* The `strcmp()` function compares the two strings `str1` and `str2`, limited by `n` */
int strncmp (const char* str1, const char *str2, size_t n);

/*
  The `strchr()` function returns a pointer to the first
  occurrence of the character `c` in the string `str`
*/
char *strchr (const char *str, int c);

char *strchrnul (const char *str, int c);

/*
  The `strnchr()` function returns a pointer to the first
  occurrence of the character `c` in the string `str`, count down from `n`
*/
char *strnchr (const char *str, int c, size_t n);

/*
  The `strncmp()` function is similar to `strcmp()`, except it
  compares only the first (at most) n bytes of `str1` and `str2`
*/
int strncmp (const char* str1, const char *str2, size_t n);

/* duplicate a string */
char *strdup (const char *str);

/* duplicate a string, limited by `n` */
char *strndup (const char *str, size_t n);

/* 
  The `memset()` function  fills  the first `n` bytes of the
  memory area pointed to by `str` with the constant byte `c`
*/
void *memset (void *dest, int c, size_t n);

/* Compare memory areas (`n` bytes) */
int memcmp (const void *ptr1, const void *ptr2, size_t n);

/*
  returns a pointer to the first occurrence of the uchar `c`
  in the space start from `ptr`. If the pointer finder holds
  run out of range, return NULL
*/
void *memchr (const void *ptr, int c, size_t n);

/*
  The `memcpy()` function  copies `n`  bytes
  from memory area `src` to memory area `dest`
*/
void *memcpy (void *dest, const void *src, size_t n);

// TODO : Other functions in string.h

#endif
