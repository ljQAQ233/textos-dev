#include "stdio.h"

int dprintf(int fd, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int i = vdprintf(fd, format, ap);
    va_end(ap);
    return i;
}
