#include <textos/dev.h>
#include <textos/args.h>
#include <textos/klib/vsprintf.h>

extern void __printkcon(char *buf);

size_t printk(const char *format, ...)
{
    char buf[256];
    va_list args;
    va_start(args, format);
    size_t i = vsprintf(buf, format, args);
    __printkcon(buf);
    va_end(args);
    return i;
}
