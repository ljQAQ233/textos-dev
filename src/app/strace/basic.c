/*
 * Basic data types
 */
#include "strace.h"

#define defint(N, T, C)         \
    struct type _tyb_##N = {    \
        .name = _STR(T),        \
        .size = sizeof(T),      \
        .cls = CLASS_INT,       \
        .printer = int_printer, \
        .INT = {.conv = C},     \
    }

defint(i, int, 'd');
defint(l, long, 'd');
defint(ll, long long, 'd');
defint(u, unsigned, 'd');
defint(ul, unsigned long, 'd');
defint(ull, unsigned long long, 'd');
defint(ix, int, 'x');
defint(lx, long, 'x');
defint(llx, long long, 'x');
defint(ux, unsigned, 'x');
defint(ulx, unsigned long, 'x');
defint(ullx, unsigned long long, 'x');
defint(io, int, 'o');
defint(lo, long, 'o');
defint(llo, long long, 'o');
defint(uo, unsigned, 'o');
defint(ulo, unsigned long, 'o');
defint(ullo, unsigned long long, 'o');
