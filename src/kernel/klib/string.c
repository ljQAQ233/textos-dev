#include <textos/mm/heap.h>

/*
 * The `strlen()` function calculates the
 * length of the string pointed to by `str`,
 * excluding the terminating null byte ('\0')
 */
size_t strlen(const char *str)
{
    size_t i = 0;

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
int strcmp(const char *str1, const char *str2)
{
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return (unsigned char)*str1 - (unsigned char)*str2;
}

/*
 * The `strchr()` function returns a pointer to the first
 * occurrence of the character `c` in the string `str`
 */
char *strchr(const char *str, int c)
{
    do {
        if (*str == (char)c) return (char *)str;
    } while (*str++);
    return 0;
}

char *strrchr(const char *str, int c)
{
    char *res = 0;
    do {
        if (*str == (char)c) res = (char *)str;
    } while (*str++);
    return res;
}

char *strchrnul(const char *str, int c)
{
    do {
        if (*str == (char)c) return (char *)str;
    } while (*str++);
    return (char *)(str - 1);
}

char *strrchrnul(const char *str, int c)
{
    const char *r = 0;
    do {
        if (*str == (char)c) {
            r = str;
        }
    } while (*str++);
    return (char *)(r ? r : str - 1);
}

char *strstr(const char *haystack, const char *needle)
{
    if (*needle == '\0') return (char *)haystack;

    for (; *haystack != '\0'; ++haystack) {
        const char *h = haystack;
        const char *n = needle;

        while (*h != '\0' && *n != '\0' && *h == *n) {
            ++h;
            ++n;
        }

        if (*n == '\0') return (char *)haystack;
    }

    return 0;
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
int strncmp(const char *str1, const char *str2, size_t n)
{
    while (*str1 && *str2 && *str1 == *str2 && n) {
        str1++;
        str2++;
        n--;
    }

    if (n == 0) return 0;
    return (unsigned char)*str1 - (unsigned char)*str2;
}

char *strcat(char *dest, const char *src)
{
    char *ret = dest;
    while (*dest)
        dest++;
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
    return ret;
}

void *memcpy(void *dest, const void *src, size_t n);

/*
 * duplicate a string (malloc-based)
 */
char *strdup(const char *str)
{
    size_t len = strlen(str);

    char *p = malloc(len + 1);
    if (p == 0) return 0;

    return memcpy(p, str, len + 1);
}

/*
 * duplicate a string, limited by `n` (malloc-based)
 */
char *strndup(const char *str, size_t n)
{
    size_t len = strnlen(str, n);
    char *p = malloc(len + 1);
    if (!p) return 0;
    memcpy(p, str, len);
    p[len] = '\0';
    return p;
}

/*
 * The `memset()` function fills the first `n` bytes of the
 * memory area pointed to by `dest` with the constant byte `c`
 */
void *memset(void *dest, int c, size_t n)
{
    unsigned char *p = dest;
    while (n-- > 0)
        *p++ = (unsigned char)c;

    return dest;
}

/*
 * Compare memory areas (`n` bytes)
 */
int memcmp(const void *ptr1, const void *ptr2, size_t n)
{
    unsigned char *p1 = (unsigned char *)ptr1;
    unsigned char *p2 = (unsigned char *)ptr2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

/*
 * returns a pointer to the first occurrence of the uchar `c`
 * in the space start from `ptr`. If the pointer finder holds
 * run out of range, return 0
 */
void *memchr(const void *ptr, int c, size_t n)
{
    unsigned char *p = (unsigned char *)ptr;
    while (n--) {
        if (*p == (unsigned char)c) {
            return (void *)p;
        }
        p++;
    }

    return 0;
}

/*
 * Copy `n`  bytes from memory area `src` to memory area `dest`
 */
void *memcpy(void *dest, const void *src, size_t n)
{
    char *p = dest;
    while (n--)
        *p++ = *(char *)src++;

    return dest;
}
