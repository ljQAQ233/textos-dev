#include "stdio.h"

int snprintf(char *buf, size_t n, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int i = vsnprintf(buf, n, format, ap);
    va_end(ap);
    return i;
}
