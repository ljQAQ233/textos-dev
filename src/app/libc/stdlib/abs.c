/*
 * special case:
 *  (gdb) p -(int)2147483648
 *  $1 = -2147483648
 */

#include <stdlib.h>

int abs(int x)
{
    return x < 0 ? -x : x;
}

long labs(long x)
{
    return x < 0 ? -x : x;
}

long long llabs(long long x)
{
    return x < 0 ? -x : x;
}
