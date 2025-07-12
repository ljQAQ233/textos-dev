#include "stdio.h"
#include <string.h>
#include <errno.h>

void perror(const char *s)
{
    char *err = strerror(errno);
    if (s && s[0])
    {
        fputs(s, stderr);
        fputc(':', stderr);
        fputc(' ', stderr);
    }
    fputs(err, stderr);
    fputc('\n', stderr);
}