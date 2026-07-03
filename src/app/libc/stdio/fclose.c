#include "stdio.h"
#include <malloc.h>
#include <unistd.h>

int __fclosex(FILE *f)
{
    fflush(f);
    f->close(f);
    if (f->fl & F_PERM) return 0;
    free(f);
    return 0;
}

int fclose(FILE *f)
{
    __ofl_del(f);
    return __fclosex(f);
}
