#include "stdio.h"

int vdprintf(int fd, const char *format, va_list ap)
{
    char dummy[1];
    FILE f = {
        .fd = fd,
        .lbf = EOF,
        .write = __stdio_write,
        .buf = dummy,
    };
    return vfprintf(&f, format, ap);
}
