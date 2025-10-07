#include <textos/textos.h>
#include <textos/args.h>

#define TMP_BUFFER_SIZE 64

enum {
  LEFT    = 1,
  SIGN    = 1 << 1,
  ZERO    = 1 << 2,
  SPECIAL = 1 << 3,
  SPACE   = 1 << 4,
};

#define is_digit(c) ('0' <= (char)c && (char)c <= '9')

static int _int(char *ptr, int *width)
{
    int i = 0;
    int l = 0;

    while (is_digit (*ptr)) {
        i = i * 10 + *ptr++ - '0';
        l++;
    }

    *width = i;

    return l;
}

static const char upstr[] = "0123456789ABCDEF";
static const char lwstr[] = "0123456789abcdef";

static int _number(char *buffer, u64 num, int base, bool upper)
{
    const char *letters = upper ? upstr : lwstr;

    int siz = 0;
    char tmp[TMP_BUFFER_SIZE];

    char *ptr = tmp;

    if (num == 0) {
        *ptr++ = '0';
        siz++;
    } else {
        while (num != 0) {
            *ptr++ = letters[num % base];
            num /= base;
            siz++;
        }
    }

    ptr--;
    int i = 0;
    while (i < siz) {
        buffer[i++] = *ptr--;
    }
    buffer[i] = '\0';

    return siz;
}

int vsprintf(char *buffer, const char *format, va_list args)
{
    char *out = buffer;
    char *ptr = (char*)format;

    int flgs;
    while (ptr && *ptr)
    {
        if (*ptr != '%')
        {
            *out++ = *ptr++;
            continue;
        }

        flgs = 0;
parse_flgs:
        ptr++;
        switch (*ptr) {
            case '#': // 与 o,x或X 一起使用时,非零值前面会分别显示 0,0x或0X
                flgs |= SPECIAL;
                goto parse_flgs;
            case '0': // 在指定填充的数字左边放置0,而不是空格
                if (flgs & ZERO) {
                    break; // 再有就是宽度
                }
                flgs |= ZERO;
                goto parse_flgs;
            case '-': // 在给定的字段宽度内左对齐,默认是右对齐
                flgs |= LEFT;
                goto parse_flgs;
            case ' ': // 如果没有写入任何符号,则在该值前面填空格
                flgs |= SPACE;
                goto parse_flgs;
            case '+': // 如果是正数,则在最前面加一个正号
                flgs |= SIGN;
                goto parse_flgs;
            default:
                break;
        }
        ptr--;

        int offset = 0;
        int radix = 10;
        int len = 0;
        int width = 0;
        bool sign = false,
             upper = false;
parse_args:
        ptr++;
        switch (*ptr)
        {
            case '%':
                *out++ = '%';
                ptr++;
                continue;
            case 'l':
            case 'L':
                len = 1;
                if (*(ptr+1) == 'l' || *(ptr+1) == 'L') {
                    ptr++;
                    len = 2;
                }
                goto parse_args;

            case 'X':
                upper = true;
            case 'x':
                radix = 16;
                break;
            case 'o':
                radix = 8;
                break;
            case 'd':
            case 'i':
                sign = true;
            case 'u':
                radix = 10;
                break;
            case 'c':
                /* Includes the char */
                if (width > 1)
                    while (--width)
                        *out++ = ' ';
                *out++ = (char)va_arg (args, int);

                ptr++;
                continue;
            case 's':
                {
                    char *src = (char *)va_arg (args, char *);
                    if (src == NULL)
                        src = "(null)"; // 如果是 NULL 就填 "(null), 总不可能去访问 0x00 吧 QaQ"
                    for (char *p = src;p && *p;p++)
                        width--;
                    while (width-- > 0)
                        *out++ = ' ';
                    while (*src)
                        *out++ = *src++;

                    ptr++;
                    continue;
                }
            case 'p':
                radix = 16;
                len = 2;
                flgs |= SPECIAL;
                break;

            case 'q':
                {
                    char fill = (char)va_arg (args, int);
                    for (int i = 0 ; i < width ; i++) *out++ = fill;
                }
                ptr++;
                continue;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '0':
                offset = _int (ptr, &width);
                ptr += offset - 1;
                goto parse_args;
            case '*':
                width = va_arg (args ,int);
                if (width < 0)
                    flgs |= LEFT; // 左对齐
                goto parse_args;
        }

        u64 val;
        bool minus = false;

        if (len == 0)
        {
            val = va_arg (args, unsigned int);
            if (sign && (int)val < 0) {
                minus = true;
                val = -(int)val; // 符号位将在最后于字符串上添上.
            }
        }
        else if (len == 1)
        {
            val = va_arg (args, unsigned long);
            if (sign && (long)val < 0) {
                minus = true;
                val = -(long)val;
            }
        }
        else if (len == 2)
        {
            val = va_arg (args, unsigned long long);
            if (sign && (long long)val < 0) {
                minus = true;
                val = -(long long)val;
            }
        }

        /* 每一次添加字符('+','-',' '...)都会导致 Siz 减小,
           这么做在于最后可以直接使用 Siz 来进行填充操作. */
        int siz = width;
        char tmp[TMP_BUFFER_SIZE];
        siz -= _number(tmp, val, radix, upper);

        char prefix = 0;
        if (radix == 10)
        {
            if (minus && siz--)
                prefix = '-';
            /* 以下是正数的情况 */
            else if (sign && flgs & SIGN && siz--)
                prefix = '+';
            else if (flgs & SPACE && siz--)
                prefix = ' ';
        }

        if (flgs & SPECIAL) {
            siz -= (radix == 16) ? 2 :
                   (radix == 8 ) ? 1 : 0;
        }

        if (flgs & ZERO)
        {
            if (prefix) *out++ = prefix;
            /* "0x" "0X" "0" for SPECIAL */
            if (flgs & SPECIAL && radix != 10)
            {
                *out++ = '0';
                if (radix == 16) {
                    *out++ = upper ? 'X' : 'x';
                }
            }
        }

        /* Padding */
        if (!(flgs & LEFT) && siz > 0)
        {
            char pad = flgs & ZERO ? '0' : ' ';

            while (siz--) {
                *out++ = pad;
            }
        }

        /* Symbol and others after padding if that is not filled with '0' -> "    0x91d" */
        if (!(flgs & ZERO))
        {
            if (prefix) *out++ = prefix;
            /* "0x" "0X" "0" for SPECIAL */
            if (flgs & SPECIAL && radix != 10)
            {
                *out++ = '0';
                if (radix == 16)
                    *out++ = upper ? 'X' : 'x';
            }
        }

        for (char *p = tmp;*p;)
            *out++ = *p++;

        ptr++;
    }
    *out = '\0';

    return (int)(out - buffer);
}

int sprintf(char *buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    int i = vsprintf(buffer, format, args);

    va_end(args);
    return i;
}
