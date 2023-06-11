#include <string.h>

/*
  The strlen() function calculates the 
  length of the string pointed to by `str`,
  excluding the terminating null byte ('\0')
*/
size_t strlen (char *str)
{
    size_t i =0;
    
    while (str && *str++)
        i++;
    return i;
}

/*
  The strcpy() function  copies  the  string
  pointed to by `src`, including the  terminating
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

/* The  strcmp() function compares the two strings `str1` and `str2` */
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
  The strchr() function returns a pointer to the first
  occurrence of the character `c` in the string `str`
*/
char *strchr (const char *str, int c)
{
    while (str && *str) {
        if (*str++ == (char)c)
            return (char *)str;
    }
    if (c == '\0') {
        return (char *)str;
    }

    return NULL;
}

/*
  The  memset()  function  fills  the first `n`  bytes  of the
  memory area pointed to by `` with the constant byte `c`
*/
void *memset (void *dest, int c, size_t n) {
    u8 *p = dest;
    while (n-- > 0)
        *p++ = (u8) c;

    return dest;
}

/*
  The  memcpy() function  copies n  bytes
  from memory area `src` to memory area `dest`
*/
void *memcpy (void *dest, const void *src, size_t n)
{
    char *p = dest;
    while (n--) {
        *p++ = *(char*)src++;
    }

    return dest;
}

// TODO : Other functions in string.h