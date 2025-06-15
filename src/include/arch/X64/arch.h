#ifndef __TYPE_ARC_H__
#define __TYPE_ARC_H__

#define ARCH_STRING "x86_64"

// stdint.h (partial)
typedef long long           __int64_t;
typedef int                 __int32_t;
typedef short               __int16_t;
typedef signed char         __int8_t;

typedef unsigned long long  __uint64_t;
typedef unsigned int        __uint32_t;
typedef unsigned short      __uint16_t;
typedef unsigned char       __uint8_t;

typedef char                __char8;
typedef unsigned short      __char16;

typedef __int64_t  __intptr_t;
typedef __int64_t  __intmax_t;
typedef __uint64_t __uintptr_t;
typedef __uint64_t __uintmax_t;

// stddef.h
typedef __uint64_t __size_t;
typedef __int64_t __ssize_t;
typedef __int64_t __ptrdiff_t;

// c11
typedef struct {
    long long __ll;
    long double __ld;
} __max_align_t;

typedef int __wchar_t;

#endif
