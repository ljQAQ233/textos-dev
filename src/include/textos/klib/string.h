#ifndef __STRING_H__
#define __STRING_H__

/*
  The strlen() function calculates the 
  length of the string pointed to by `str`,
  excluding the terminating null byte ('\0')
*/
size_t strlen (char *str);

/*
  The strcpy() function  copies  the  string
  pointed to by `src`, including the  terminating
  null byte ('\0')
*/
char *strcpy (char *dest, const char *src);

/* The  strcmp() function compares the two strings `str1` and `str2` */
int strcmp (const char* str1,const char *str2);

/*
  The strchr() function returns a pointer to the first
  occurrence of the character `c` in the string `str`
*/
char *strchr (const char *str, int c);

/* 
  The  memset()  function  fills  the first `n`  bytes  of the
  memory area pointed to by `str` with the constant byte `c`
*/
void *memset (void *dest, int c, size_t n);

/*
  The  memcpy() function  copies n  bytes
  from memory area `src` to memory area `dest`
*/
void *memcpy (void *dest, const void *src, size_t n);

// TODO : Other functions in string.h

#endif