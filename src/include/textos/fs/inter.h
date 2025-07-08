#pragma once

#include <textos/dev.h>

typedef struct _packed
{
    u8   bootable         ; // Bootable (Active) -> 0x80
    u8   start_head       ;
    u16  start_sec     :6 ;
    u16  start_clinder :10;
    u8   sysid            ;
    u8   end_head         ;
    u16  end_sec       :6 ;
    u16  end_clinder   :10;
    u32  relative         ;
    u32  total            ;
} part_t;

typedef struct _packed
{
    u8      others[446];
    part_t  ptab[4];
    u16     endsym;
} mbr_t;

// utils

#define _UTIL_NEXT()                    \
    static inline char *_next (char *p) \
    {                                   \
        while (*p != '/' && *p)         \
            p++;                        \
        while (*p == '/')               \
            p++;                        \
        return p;                       \
    }                                   \

#define _UTIL_PATH_DIR()                    \
    static inline bool _path_isdir (char *p) \
    {                                       \
        bool res = false;                   \
        for ( ; p && *p ; p++) {            \
            if (*p == '/')                  \
                res = true;                 \
            if (*p == ' ')                  \
                continue;                   \
            res = false;                    \
        }                                   \
        return res;                         \
    }

#define _UTIL_CMP()                                       \
    static bool _cmp(char *A, char *B)                    \
    {                                                     \
        size_t lenA = MIN(strlen(A), strchr(A, '/') - A); \
        size_t lenB = MIN(strlen(B), strchr(B, '/') - B); \
        if (lenA != lenB) return false;                   \
        for (size_t i = 0; i < lenB; i++)                 \
            if (A[i] != B[i]) return false;               \
        return true;                                      \
    }

extern void noopt_handler();
