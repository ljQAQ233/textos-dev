#include "stdio.h"

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int i = vprintf(format, args);
    va_end(args);
    return i;
}
