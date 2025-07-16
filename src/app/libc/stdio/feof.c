#include "stdio.h"

int feof(FILE *f)
{
    return (f->fl & F_EOF) == F_EOF;
}
