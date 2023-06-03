#include <textos/textos.h>
#include <textos/args.h>
#include <textos/console.h>

#define BUFFER_MAX 32

static char letters[] = "0123456789abcdef0123456789ABCDEF";

#define PRINT_BUFFER_MAX 256

static int buf_idx;
static char buf[256];

static void reset ()
{
    buf_idx = 0;
}

static void save_chr (char Char)
{
    buf[buf_idx++] = Char;
}

static void save (char *str)
{
    while (str && *str && buf_idx < PRINT_BUFFER_MAX) {
        buf[buf_idx++] = *str++;
    }
}

static void _print_hex (u64 n)
{
    char tmp[BUFFER_MAX] = {0};
    int i = BUFFER_MAX - 1;
    while (n != 0) {
        tmp[--i] = letters[n % 16];
        n /= 16;
    }
    save (&tmp[i]);
}

static void _print_hexup (u64 n)
{
    char tmp[BUFFER_MAX] = {0};
    int i = BUFFER_MAX - 1;
    while (n != 0) {
        tmp[--i] = letters[(n % 16) + 16];
        n /= 16;
    }

    save (&tmp[i]);
}

static void _print_u64 (u64 n)
{
    char tmp[BUFFER_MAX] = {0};
    int i = BUFFER_MAX - 1;
    while (n != 0) {
        tmp[--i] = letters[n % 10];
        n /= 10;
    }
    save (&tmp[i]);
}

static void _print_i64 (int64 n)
{
    bool negative = n < 0 ? true : false;
    n = ABS(n);

    char tmp[BUFFER_MAX] = {0};
    int i = BUFFER_MAX - 1;
    while (n != 0) {
        tmp[--i] = letters[n % 10];
        n /= 10;
    }
    if (negative) {
        tmp[--i] = '-';
    }

    save (&tmp[i]);
}

static void _print_i32 (int32 n)
{
    bool negative = n < 0 ? true : false;
    n = ABS(n);

    char tmp[BUFFER_MAX] = {0};
    int i = BUFFER_MAX - 1;
    while (n != 0) {
        tmp[--i] = letters[n % 10];
        n /= 10;
    }
    if (negative) {
        tmp[--i] = '-';
    }

    save (&tmp[i]);
}

static int _printk (char *format, va_list args)
{
    reset();

    while (*format) {
        if (*format == '%') {
parse:
            switch (*++format)
            {
                case '%':
                    save_chr ('%');
                    break;

                case 'x':
                    _print_hex (va_arg (args, u64));
                    format++;
                    continue;

                case 'X':
                    _print_hexup (va_arg (args, u64));
                    format++;
                    continue;

                case 'd':
                    if (*(format - 1) == 'l'
                            && *(format - 2) == 'l') { 
                        _print_i64 (va_arg (args, int64));
                    } else {
                        _print_i32 (va_arg (args, int32));
                    }
                    format++;
                    continue;

                case 'u':
                    _print_u64 (va_arg (args, u64));
                    format++;
                    continue;
                case 'l':
                    goto parse;
            }
        }
        save_chr (*format++);
    }

    int i = 0;
    while (buf[i++]);

    return i;
}

int printk (char *format, ...)
{
    va_list args;
    va_start (args, format);
    
    int i = _printk (format, args);

    console_write (buf);

    va_end (args);

    return i;
}

