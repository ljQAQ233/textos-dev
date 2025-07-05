#include <stdlib.h>

div_t div(int num, int den)
{
    return (div_t){num / den, num % den};
}

ldiv_t ldiv(long num, long den)
{
    return (ldiv_t){num / den, num % den};
}

lldiv_t lldiv(long long num, long long den)
{
    return (lldiv_t){num / den, num % den};
}