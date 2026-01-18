#define _FILE_OFFSET_BITS 64
#include "stdio.h"

int fseek(FILE *f, long offset, int whence)
{
    return fseeko(f, offset, whence);
}
