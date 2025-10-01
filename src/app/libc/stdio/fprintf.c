#include "stdio.h"

int fprintf(FILE *f, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int i = vfprintf(f, format, ap);
    va_end(ap);
    return i;
}
