#include <textos/args.h>
#include <textos/printk.h>
#include <textos/klib/vsprintf.h>

static char buf[128];

void debugk (
        const char *file,
        const u64  line,
        const char *format,
        ...
        )
{
    va_list args;
    va_start (args, format);

    vsprintf (
            buf,
            format,
            args
        );

    printk ("[%s:%d] %s", file, line, buf);

    va_end (args);
}

