#include <stdlib.h>     

 int mkostemp(char *template, int flags)
{
    return mkostemps(template, 0, flags);
}

