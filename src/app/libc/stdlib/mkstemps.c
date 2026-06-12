#include <stdlib.h>     

int mkstemps(char *template, int suffixlen)
{
    return mkostemps(template, suffixlen, 0);
}

