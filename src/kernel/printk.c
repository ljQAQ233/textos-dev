#include <textos/dev.h>
#include <textos/args.h>
#include <textos/klib/vsprintf.h>

size_t printk (const char *format, ...)
{
    char buf[256];

    va_list args;
    va_start(args, format);

    size_t i = vsprintf(buf, format, args);
    dev_t *console = dev_lookup_type(DEV_KNCON, 0);
    dev_t *serial  = dev_lookup_type(DEV_SERIAL, 0);
    console->write(console, buf, -1);
    serial->write(serial, buf, -1);

    va_end(args);
    return i;
}

