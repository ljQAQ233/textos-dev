#include <stdlib.h>

int mkstemp(char *template)
{
    return mkostemps(template, 0, 0);
}
