#include "stdio.h"

void clearerr(FILE *f)
{
    f->fl &= ~(F_EOF | F_ERR);
}
