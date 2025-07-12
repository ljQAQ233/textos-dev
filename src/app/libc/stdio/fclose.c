#include "stdio.h"
#include <malloc.h>
#include <unistd.h>

int fclose(FILE *f)
{
    fflush(f);
    __ofl_del(f);
    f->close(f);
    if (f->fl & F_PERM)
        return 0;
    free(f);
    return 0;
}