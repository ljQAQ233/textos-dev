#include <stdint.h>
#include <string.h>
#include <malloc.h>

typedef uint8_t u8;

#define max(A,B) (((A) > (B)) ? (A) : (B))
#define min(A,B) (((A) < (B)) ? (A) : (B))

/*
 * The `strlen()` function calculates the 
 * length of the string pointed to by `str`,
 * excluding the terminating null byte ('\0')
*/
size_t strlen(const char *str)
{
    size_t i =0;
    
    while (str && *str++)
        i++;
    return i;
}

/*
 * `strcpy()` copies the string pointed to by `src`,
 * including the terminating null byte ('\0')
*/
char *strcpy(char *dest, const char *src)
{
    char *p = dest;
    while (*src)
        *p++ = *src++;
    *p++ = *src++; // copy '\0'

    return dest;
}

/*
 * `strcmp()` compares the two strings `str1` and `str2`
 */
int strcmp(const char* str1,const char *str2)
{
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return (u8)*str1 - (u8)*str2;
}

/*
 * The `strchr()` function returns a pointer to the first
 * occurrence of the character `c` in the string `str`
*/
char *strchr(const char *str, int c)
{
    while (*str) {
        if (*str == (char)c)
            return (char *)str;
        str++;
    }
    if (c == '\0')
        return (char *)str;

    return NULL;
}

char *strchrnul (const char *str, int c)
{
    while (*str) {
        if (*str == (char)c)
            return (char *)str;
        str++;
    }

    return (char *)str;
}

char *strstr(const char *haystack, const char *needle)
{
    if (*needle == '\0')
        return (char *)haystack;

    for (; *haystack != '\0'; ++haystack)
    {
        const char *h = haystack;
        const char *n = needle;

        while (*h != '\0' && *n != '\0' && *h == *n)
        {
            ++h;
            ++n;
        }

        if (*n == '\0')
            return (char *)haystack;
    }

    return NULL;
}

/*
 * Limited by `n`, which is similar to `strcpy()`
 */
char *strncpy(char *dest, const char *src, size_t n)
{
    char *p = dest;
    while (*src && n--)
        *p++ = *src++;
    while (n--)
        *p++ = '\0'; // put '\0'

    return dest;
}

/*
 * `strncmp()` is similar to `strcmp()`, except it
 * compares only the first (at most) n bytes of `str1` and `str2`
*/
int strncmp(const char* str1, const char *str2, size_t n)
{
    while (*str1 && *str2 && *str1 == *str2 && n) {
        str1++;
        str2++;
        n--;
    }

    if (n == 0)
        return 0;
    return (u8)*str1 - (u8)*str2;
}

char *strcat(char* dest, const char* src)
{
    char* ret = dest;
    while (*dest)
        dest++;
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
    return ret;
}

/*
 * duplicate a string (malloc-based)
 */
char *strdup(const char *str)
{
    size_t len = strlen(str);

    char *p = malloc(len + 1);
    if (p == NULL)
        return NULL;

    return memcpy(p, str, len + 1);
}

/*
 * duplicate a string, limited by `n` (malloc-based)
 */
char *strndup(const char *str, size_t n)
{
    size_t cpy = min(strlen(str) + 1, n);  // num of characters to copy
    size_t bfs = max(strlen(str) + 1, n);  // real buffer size

    char *p = malloc(bfs);
    if (p == NULL)
        return NULL;
    
    return memcpy(p, str, cpy);
}

/*
 * The `memset()` function fills the first `n` bytes of the
 * memory area pointed to by `dest` with the constant byte `c`
*/
void *memset(void *dest, int c, size_t n) {
    u8 *p = dest;
    while (n-- > 0)
        *p++ = (u8) c;

    return dest;
}

/*
 * Compare memory areas (`n` bytes)
 */
int memcmp(const void *ptr1, const void *ptr2, size_t n)
{
    u8 *p1 = (u8 *)ptr1;
    u8 *p2 = (u8 *)ptr2;
    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

/*
 * returns a pointer to the first occurrence of the uchar `c`
 * in the space start from `ptr`. If the pointer finder holds
 * run out of range, return NULL
*/
void *memchr(const void *ptr, int c, size_t n)
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

/*
 * Copy `n`  bytes from memory area `src` to memory area `dest`
 */
void *memcpy(void *dest, const void *src, size_t n)
{
    char *p = dest;
    while (n--)
        *p++ = *(char*)src++;

    return dest;
}

extern char *__get_errstr(int nr);

/*
 * This string must not be modified by the application...
*/
char *strerror(int errnum)
{
    char *str = __get_errstr(errnum);
    return str;
}

// TODO : Other functions in string.h
