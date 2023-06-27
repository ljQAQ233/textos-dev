#include <io.h>
#include <textos/debug.h>
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

int dmask = 0;

int dprintk_set(int mask)
{
    int old = dmask;

    mask &= K_ALL; // drop
    dmask = mask;
    
    return old;
}

// dprint

static void dvprintk(int lv, const char *format, va_list ap)
{
    char buf[128];
    int mask = dmask | K_SYNC;
    if (mask & lv) {
        vsprintf(buf, format, ap);
        dputs(buf);
        if (lv & K_SYNC)
            printk(buf);
    }
}

void dprintk(int lv, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    dvprintk(lv, format, args);

    va_end(args);
}

void debugk(int lv, const char *file, const int line, const char *format, ...)
{
    char buf[128];
    va_list args;
    va_start(args, format);

    vsprintf(buf, format, args);
    dprintk(lv, "[%s:%d] %s", file, line, buf);

    va_end(args);
}

