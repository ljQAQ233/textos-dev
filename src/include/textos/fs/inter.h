#pragma once

#include <textos/dev.h>

#define FS_INITIALIZER(name) \
        void *name(dev_t *hd, mbr_t *mbr, part_t *pentry)

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
    }                                       \

#define CKDIR(n) ((n)->attr & NA_DIR)

#define CKFILE(n) (~((n)->attr) & NA_DIR)

extern void noopt_handler();
