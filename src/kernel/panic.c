#include <cpu.h>
#include <textos/args.h>
#include <textos/debug.h>
#include <textos/klib/vsprintf.h>

#include <textos/printk.h>

void panic(const char *file, const u64 line, const char *format, ...)
{
    char buf[128];
    va_list args;
    va_start(args, format);

    vsprintf(buf, format, args);
    dprintk(K_SYNC, "[%s:%d] %s", file, line, buf);

    va_end(args);

    __asm__ volatile(
        "cli\n"
        "hlt\n"
    );
}
