#ifndef __PANIC_H__
#define __PANIC_H__

void panic(const char *file, const u64 line, const char *format, ...);

#define PANIC(format, ARGS...) \
        panic(__FILE__, __LINE__, format, ##ARGS)

// 可选方案:
/*
#include <cpu.h>
#define panic(file, line, format, ARGS)                        \
        do {                                                   \
            debugk (file, line, "Panic!!! ->" format, ##ARGS)  \
            while (true) Halt();                               \
        while (false);
*/

#endif
