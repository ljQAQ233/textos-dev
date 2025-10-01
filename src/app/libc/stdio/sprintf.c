#include "stdio.h"
#include <limits.h>

int sprintf(char *buf, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int i = vsnprintf(buf, INT_MAX, format, ap);
    va_end(ap);
    return i;
}
