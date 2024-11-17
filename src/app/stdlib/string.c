#include <string.h>

/*
  The `strlen()` function calculates the 
  length of the string pointed to by `str`,
  excluding the terminating null byte ('\0')
*/
size_t strlen (const char *str)
{
    size_t i =0;
    
    while (str && *str++)
        i++;
    return i;
}

/*
  The `strcpy()` function copies the string
  pointed to by `src`, including the terminating
  null byte ('\0')
*/
char *strcpy (char *dest, const char *src)
{
    char *p = dest;
    while (p && src && *src)
        *p++ = *src++;
    *p++ = *src++; // copy '\0'

    return dest;
}

/* The `strcmp()` function compares the two strings `str1` and `str2` */
int strcmp (const char* str1,const char *str2)
{
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 == *str2 ? 0
         : *str1 >  *str2 ? 1 : -1;
}

/*
  The `strchr()` function returns a pointer to the first
  occurrence of the character `c` in the string `str`
*/
char *strchr (const char *str, int c)
{
    while (str && *str) {
        if (*str == (char)c)
            return (char *)str;
        str++;
    }
    if (c == '\0') {
        return (char *)str;
    }

    return NULL;
}

char *strchrnul (const char *str, int c)
{
    while (str && *str) {
        if (*str == (char)c)
            return (char *)str;
        str++;
    }
    if (c == '\0') {
        return (char *)str;
    }

    return (char *)str;
}

/* Limited by `n`, is similar to `strcpy()` */
char *strncpy (char *dest, const char *src, size_t n)
{
    char *p = dest;
    while (p && src && *src && n--)
        *p++ = *src++;
    *p = '\0'; // put '\0'

    return dest;
}

/*
  The `strncmp()` function is similar to `strcmp()`, except it
  compares only the first (at most) n bytes of `str1` and `str2`
*/
int strncmp (const char* str1, const char *str2, size_t n)
{
    while (*str1 && *str2 && *str1 == *str2 && n--) {
        str1++;
        str2++;
    }
    return *str1 == *str2 ? 0
         : *str1 >  *str2 ? 1 : -1;
}

/*
  The `strnchr()` function returns a pointer to the first
  occurrence of the character `c` in the string `str`, count down from `n`
*/
char *strnchr (const char *str, int c, size_t n)
{
    while (str && *str && n--) {
        if (*str == (char)c)
            return (char *)str;
        str++;
    }
    if (c == '\0') {
        return (char *)str;
    }

    return NULL;
}

#include <textos/mm.h>

/* duplicate a string */
char *strdup (const char *str)
{
    size_t len = strlen(str);

    char *p = malloc(len + 1);
    if (p == NULL)
        return NULL;

    return memcpy (p, str, len + 1);
}

/* duplicate a string, limited by `n` */
char *strndup (const char *str, size_t n)
{
    size_t cpy = MIN (strlen(str) + 1, n);  // num of characters to copy
    size_t bfs = MAX (strlen(str) + 1, n);  // real buffer size

    char *p = malloc(bfs);
    if (p == NULL)
        return NULL;
    
    return memcpy (p, str, cpy);
}

/*
  The `memset()` function fills the first `n` bytes of the
  memory area pointed to by `dest` with the constant byte `c`
*/
void *memset (void *dest, int c, size_t n) {
    u8 *p = dest;
    while (n-- > 0)
        *p++ = (u8) c;

    return dest;
}

/* Compare memory areas (`n` bytes) */
int memcmp (const void *ptr1, const void *ptr2, size_t n)
{
    char *p1 = (char *)ptr1;
    char *p2 = (char *)ptr2;

    while (*p1 == *p2 && n--)
    return *p1 == *p2 ? 0
         : *p1 >  *p2 ? 1 : -1;
}

/*
  returns a pointer to the first occurrence of the uchar `c`
  in the space start from `ptr`. If the pointer finder holds
  run out of range, return NULL
*/
void *memchr (const void *ptr, int c, size_t n)
{
    u8 *p = (u8 *)ptr;
    while (n--)
    {
        if (*p == (u8)c)
            return (void *)p;
        p++;
    }

    return NULL;
}

/* Copy `n`  bytes from memory area `src` to memory area `dest` */
void *memcpy (void *dest, const void *src, size_t n)
{
    char *p = dest;
    while (n--) {
        *p++ = *(char*)src++;
    }

    return dest;
}

// TODO : Other functions in string.h
