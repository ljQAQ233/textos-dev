#include <textos/args.h>
#include <textos/debug.h>
#include <textos/console.h>
#include <textos/klib/vsprintf.h>

size_t printk(const char *format, ...)
{
    char buf[256];

    va_list args;
    va_start(args, format);

    size_t i = vsprintf(buf, format, args);
    console_write(buf);

    va_end(args);
    return i;
}

