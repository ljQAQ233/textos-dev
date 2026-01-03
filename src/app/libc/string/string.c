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
 * POSIX.1-2008
 */
size_t strnlen(const char *str, size_t maxlen)
{
    size_t i = 0;
    
    while (str && i < maxlen && *str++)
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
    do {
        if (*str == (char)c)
            return (char *)str;
    } while (*str++);
    return NULL;
}

char *strrchr(const char *str, int c)
{
    char *res = NULL;
    do {
        if (*str == (char)c)
            res = (char *)str;
    } while (*str++);
    return res;
}

char *strchrnul(const char *str, int c)
{
    do {
        if (*str == (char)c)
            return (char *)str;
    } while (*str++);
    return (char *)(str - 1);
}

char *strrchrnul(const char *str, int c)
{
    const char *r = NULL;
    do {
        if (*str == (char)c)
            r = str;
    } while (*str++);
    return (char *)(r ? r : str - 1);
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

// string span
// 从字符串开头开始, 连续有多少个字符都属于给定字符集合
size_t strspn(const char *str, const char *accept)
{
    unsigned char table[256] = {0};
    unsigned char *s = (unsigned char *)accept;
    while (*s)
        table[*s++] = 1;
    s = (unsigned char *)str;
    while (*s && table[*s])
        s++;
    return (size_t)((const char *)s - str);
}

// string complement span
// 从字符串开头开始, 连续有多少个字符「不在」给定集合里
size_t strcspn(const char *str, const char *reject)
{
    unsigned char table[256] = {0};
    unsigned char *s = (unsigned char *)reject;
    while (*s)
        table[*s++] = 1;
    s = (unsigned char *)str;
    while (*s && !table[*s])
        s++;
    return (size_t)((const char *)s - str);
}

char *strpbrk(const char *str, const char *accept)
{
    str += strcspn(str, accept);
    return *str ? (char *)str : 0;
}

/*
 * Limited by `n`, which is similar to `strcpy()`
 */
char *strncpy(char *dest, const char *src, size_t n)
{
    char *p = dest;
    while (*src && n)
        *p++ = *src++, n--;
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
    size_t len = strnlen(str, n);
    char *p = malloc(len + 1);
    if (!p)
        return NULL;
    memcpy(p, str, len);
    p[len] = '\0';
    return p;
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
