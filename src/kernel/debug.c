#include <io.h>
#include <textos/args.h>
#include <textos/printk.h>
#include <textos/klib/vsprintf.h>

#define R_CON 0xe9

static inline void dputc(char c)
{
    outb(R_CON, c);
}

static void dputs(char *str)
{
    char *p = str;
    while (*p)
        dputc(*p++);
}

// dprint

static void dvprintk(const char *format, va_list ap)
{
    char buf[256];
    vsprintf(buf, format, ap);
    dputs(buf);
}

void dprintk(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    dvprintk(format, args);
    va_end(args);
}
