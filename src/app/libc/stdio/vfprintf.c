#include "stdio.h"
#include <malloc.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "big.h"

enum
{
    LEFT = 1,
    SIGN = 2,
    ZERO = 4,
    SPECIAL = 8,
    SPACE = 16,
};

#define out(c)                 \
    do {                       \
        int ret = fputc(c, f); \
        if (ret < 0) return n; \
        n++;                   \
    } while (0)

static void stoint(const char **ptr, int *r)
{
    int n = 0;
    const char *p = *ptr;
    while (*p == '0')
        p++;
    while ('0' <= *p && *p <= '9')
        n = n * 10 + *p++ - '0';
    *r = n;
    *ptr = p;
}

static int fmt_num(char *buf, char *prefix, int len, char spec, int flgs,
                   va_list *ap, int *size)
{
    uint64_t v;
    bool minus = false;
    bool sign = spec == 'i' || spec == 'd';
    static const char upstr[] = "0123456789ABCDEF";
    static const char lwstr[] = "0123456789abcdef";
    const char *letters = spec & 32 ? lwstr : upstr;
    int base = (spec | 32) == 'x' ? 16 : spec == 'o' ? 8 : 10;

    // clang-format off
#define X(L, T)                         \
    (len == L) {                        \
        v = sizeof(T) < 4               \
          ? va_arg(*ap, int)            \
          : va_arg(*ap, T);             \
        if (sign && (T)v < 0)           \
            minus = true, v = -(T)v; }
    if X(-2, char)
    else if X(-1, short)
    else if X(0, int)
    else if X(1, long)
    else if X(2, long long);
#undef X
    // clang-format on

    if (flgs & SPECIAL) {
        strcpy(prefix, spec == 'x'   ? "0x"
                       : spec == 'X' ? "0X"
                       : spec == 'o' ? "0"
                                     : "");
    }

    int ndigits = 0;
    if (v == 0) {
        ndigits = 1;
        buf[0] = '0';
    } else {
        for (uint64_t n = v; n; n /= base)
            ndigits++;
        for (int i = 0; i < ndigits; i++, v /= base)
            buf[ndigits - i - 1] = letters[v % base];
    }
    buf[ndigits] = 0;
    *size = ndigits;
    return minus ? -1 : 1;
}

// binary64
static void xfp_double(void *p, uint64_t *sign, uint64_t *expo, uint64_t *frac)
{
    uint64_t u64 = *(uint64_t *)p;
    *sign = u64 >> 63;
    *expo = (u64 >> 52) & 0x7ff;
    *frac = u64 & 0xfffffffffffff;
}

// do dragon4-like algorithm
static int fmt_fp(char **fpbuf, char *prefix, int len, char spec, int flgs,
                  va_list *ap, int *size, int *prec,
                  void (*xfp)(void *, uint64_t *, uint64_t *, uint64_t *))
{
    static const big pow2_52 = {
        16, "\x4\x5\x0\x3\x5\x9\x9\x6\x2\x7\x3\x7\x0\x4\x9\x6"};
    int dot = 0, e2;
    double v_d;
    uint64_t sign, expo, frac;

    if (len == 0) {
        double v = va_arg(*ap, double);
        xfp(&v, &sign, &expo, &frac);
    } else {
        long double v = va_arg(*ap, long double);
        xfp(&v, &sign, &expo, &frac);
    }

    big_new(bigfrac);
    big_push_u64(&bigfrac, frac);
    big_pad(&bigfrac, pow2_52.len);
    big_add(&bigfrac, &pow2_52);

    big_new(result);
    e2 = expo - 1023 - 52;
    if (e2 < 0) {
        // 因为 除以的是 2 的幂, 这个大整数最终肯定可以除完, 这时候 div 会返回,
        // 也就意味着, 计算结束后 prec 可能仍然没有 "满足", 这时候需要在末尾填上
        // 0. fmp_fp 返回 prec 是还剩余的 0 的个数.
        big_a_div_pow2(&bigfrac, -e2, &result, &dot, *prec);
        *prec -= result.len - dot;
    } else {
        big_a_mul_pow2(&bigfrac, e2, &result);
    }
    *prefix = 0;
    // *prec > 0, 在 *fpbuf 输出之后结果需要补 0. 考虑 1.0, *fpbuf = "1", 显然,
    // 如果没有小数点, 这个结果就是错误的, 于是小数点刚好落在末尾 时特殊处理.
    *size = big_tostr(fpbuf, &result, dot);
    if (*prec > 0 && dot == result.len) {
        (*fpbuf)[(*size)++] = '.';
        (*fpbuf)[(*size)] = '\0';
    }
    return sign ? -1 : 1;
}

#define pad_left()          \
    if (!(flgs & LEFT))     \
        while (width-- > 0) \
            out(' ');
#define pad_right()         \
    if (flgs & LEFT)        \
        while (width-- > 0) \
            out(' ');

// 下列 label:  |  flgs |       args          |
// % [parameter] [flags] [width] [.prec] [len] specifier
// ↑         ↑       ↑       ↑       ↑     ↑        ↑
// 开始符 参数位置 标志位 字段宽度 精度 长度修饰符 转换说明符
int vfprintf(FILE *f, const char *format, va_list _ap)
{
    int n = 0;
    const char *fmt = (char *)format;
    va_list ap; // we need a pointer to it
    va_copy(ap, _ap);

    while (*fmt) {
        if (*fmt != '%') {
            char *nxt = strchr(fmt, '%');
            if (!nxt) nxt = (char *)fmt + strlen(fmt);
            int ret = fwrite(fmt, 1, nxt - fmt, f);
            if (ret < 0) return n;
            n += ret;
            fmt += ret;
            continue;
        }

        int flgs = 0;
    flag:
        fmt++;
        switch (*fmt) {
        case '#': // 与 o,x或X 一起使用时,非零值前面会分别显示 0,0x或0X
            flgs |= SPECIAL;
            goto flag;
        case '0': // 在指定填充的数字左边放置0,而不是空格
            if (flgs & ZERO) {
                break; // 再有就是宽度
            }
            flgs |= ZERO;
            goto flag;
        case '-': // 在给定的字段宽度内左对齐,默认是右对齐
            flgs |= LEFT;
            goto flag;
        case ' ': // 如果没有写入任何符号,则在该值前面填空格
            flgs |= SPACE;
            goto flag;
        case '+': // 如果是正数,则在最前面加一个正号
            flgs |= SIGN;
            goto flag;
        default:
            break;
        }

        // TODO: tmp needs to hold float
        char *s, prefix[4], tmp[32], *fptmp = 0;
        int len = 0, prec = 6, width = 0;
        bool gotdot = false;
    args:
        switch (*fmt) {
        case '%':
            out(*fmt++);
            continue;
        case 'h':
        case 'H':
            len--, fmt++;
            goto args;
        case 'l':
        case 'L':
            len++, fmt++;
            goto args;

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
            stoint(&fmt, gotdot ? &prec : &width);
            goto args;
        case '.':
            gotdot = true;
            fmt++;
            goto args;
        case '*':
            if (gotdot) {
                prec = va_arg(ap, int);
            } else {
                width = va_arg(ap, int);
                if (width < 0) {
                    flgs |= LEFT;
                    width = -width;
                }
            }
            fmt++;
            goto args;
        default:
            break;
        }

        int sign;
        int size;
        switch (*fmt | 32) {
        case 'c':
            if (width > 1) pad_left();
            out((char)va_arg(ap, int));
            if (width > 1) pad_right();
            fmt++;
            continue;
        case 's': {
            char *src = (char *)va_arg(ap, char *);
            if (src == NULL) src = "(null)";
            for (char *p = src; p && *p; p++)
                width--;
            pad_left();
            while (*src)
                out(*src++);
            pad_right();
            fmt++;
            continue;
        }
        case 'x':
        case 'o':
        case 'd':
        case 'i':
        case 'u':
            sign = fmt_num(s = tmp, prefix, len, *fmt++, flgs, &ap, &size);
            break;
        case 'p':
            // FIXME: in other architectures, not long long
            sign = fmt_num(s = tmp, prefix, 2, 'x', flgs | SPECIAL, &ap, &size);
            fmt++;
            break;
        case 'g':
        case 'e':
        case 'f':
        case 'a':
            sign = fmt_fp(&fptmp, prefix, len, *fmt++, flgs, &ap, &size, &prec,
                          xfp_double);
            s = fptmp;
            break;

        default:
            continue;
        }

        size += strlen(prefix);
        bool left = flgs & LEFT;
        char pad = flgs & ZERO ? '0' : ' ';
        int padsz = fptmp ? width - size - prec : width - size;
        if (pad == ' ' && !left)
            while (padsz-- > 0)
                out(pad);
        if (sign < 0)
            out('-');
        else if (flgs & SIGN)
            out('+');
        else if (flgs & SPACE)
            out(' ');
        fputs(prefix, f);
        if (pad == '0')
            while (padsz-- > 0)
                out(pad);
        fputs(s, f);
        if (fptmp)
            while (prec-- > 0)
                out('0');
        if (pad == ' ' && left)
            while (padsz-- > 0)
                out(pad);
        n += size;
    }

    return n;
}

#if 0
void __test_vfprintf_big()
{
    double d = 3.1415926535;
    fmt_fp(stderr, &d, 1000, xfp_double);
}
#endif
