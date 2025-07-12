#include "stdio.h"

int puts(const char *s)
{
    if (!s)
        s = "(null)";
    if (fputs(s, stdout) < 0)
        return EOF;
    if (fputc('\n', stdout) < 0)
        return EOF;
    return 0;
}