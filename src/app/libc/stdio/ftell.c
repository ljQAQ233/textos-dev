#include "stdio.h"
#include <errno.h>
#include <unistd.h>
#include <limits.h>

extern off_t ftello(FILE *f);

long ftell(FILE *f)
{
    off_t pos = ftello(f);
    if (pos > LONG_MAX)
    {
        errno = EOVERFLOW;
        return -1;
    }
    return pos;
}
