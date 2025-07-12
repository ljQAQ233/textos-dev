#include "stdio.h"
#include <string.h>

int fputs(const char *restrict s, FILE *restrict f)
{
    size_t l = strlen(s);
    if (fwrite(s, 1, l, f) != l)
        return EOF;
    return 0;
}