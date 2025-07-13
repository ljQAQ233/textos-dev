#include "stdio.h"
#include <string.h>

/**
 * @brief writes the string s to stream, without its terminating null byte ('\0').
 * 
 * @return int nonnegative value if scuuess, or EOF on error
 */
int fputs(const char *restrict s, FILE *restrict f)
{
    size_t l = strlen(s);
    if (fwrite(s, 1, l, f) != l)
        return EOF;
    return 0;
}
