#include <string.h>

size_t strxfrm(char *dest, const char *src, size_t n)
{
    size_t len = 0;
    while (src[len] != '\0')
        len++;

    if (n > 0) {
        size_t i;
        for (i = 0; i < n - 1 && src[i] != '\0'; i++)
            dest[i] = src[i];
        dest[i] = '\0';
    }
    return len;
}
