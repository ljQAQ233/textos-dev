#ifndef __PANIC_H__
#define __PANIC_H__

#include <cpu.h>
#include <textos/debug.h>

#define panic(format, ARGS...)           \
    do {                                 \
        DEBUGK(K_FATAL, format, ##ARGS); \
        while (true)                     \
            halt();                      \
    } while (0)
#define PANIC(format, ARGS...) panic(format, ##ARGS)

#endif
