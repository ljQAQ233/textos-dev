#ifndef _LIMITS_H
#define _LIMITS_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#if '\xff' > 0
    #define CHAR_MIN 0
    #define CHAR_MAX 255
#else
    #define CHAR_MIN (-128)
    #define CHAR_MAX 127
#endif

#define CHAR_BIT 8
#define SCHAR_MIN (-128)
#define SCHAR_MAX 127
#define UCHAR_MAX 255
#define SHRT_MIN (-1 - 0x7fff)
#define SHRT_MAX 0x7fff
#define USHRT_MAX 0xffff
#define INT_MIN (-1 - 0x7fffffff)
#define INT_MAX 0x7fffffff
#define UINT_MAX 0xffffffffU
#define LONG_MIN (-LONG_MAX - 1)
#define ULONG_MAX (2UL * LONG_MAX + 1)
#define LLONG_MIN (-LLONG_MAX - 1)
#define LLONG_MAX 0x7fffffffffffffffLL
#define ULLONG_MAX (2ULL * LLONG_MAX + 1)

#include <bits/arch.h>

// for compact - some programs use those in compile-time
// preprocessing statement. here with a generic fallback
#ifdef __LONG_MAX__
    #define LONG_MAX __LONG_MAX__
#else
    #define LONG_MAX (~(1L << (sizeof(long) * 8 - 1)))
#endif

#define MB_LEN_MAX 4

#define NAME_MAX 255

__END_DECLS

#endif
