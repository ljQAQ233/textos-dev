#pragma once

#include <textos/args.h>

struct fs_openctx;

#define dev_get_openctx(last_arg)            \
    ({                                       \
        va_list ap;                          \
        va_start(ap, (last_arg));            \
        struct fs_openctx *openctx = /*nxt*/ \
            va_arg(ap, struct fs_openctx *); \
        va_end(ap);                          \
        openctx;                             \
    })
