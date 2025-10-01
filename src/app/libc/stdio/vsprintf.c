#include "stdio.h"
#include <limits.h>

int vsprintf(char *buf, const char *format, va_list ap)
{
    return vsnprintf(buf, INT_MAX, format, ap);
}
