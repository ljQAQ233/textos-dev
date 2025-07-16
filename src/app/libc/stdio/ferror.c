#include "stdio.h"

int ferror(FILE *f)
{
    return (f->fl & F_ERR) == F_ERR;
}
