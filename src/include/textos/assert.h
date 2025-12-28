#ifndef __ASSERT_H__
#define __ASSERT_H__

#include <textos/panic.h>

#ifndef CONFIG_RELEASE
    #define assert(expr)                \
        do {                            \
            if (!(expr)) {              \
                PANIC("assert failed"); \
            }                           \
        } while (0)
#endif

#define ASSERTK(expr) assert(expr)

#endif
