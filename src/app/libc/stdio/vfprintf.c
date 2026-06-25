#include "stdio.h"
#include <limits.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "big.h"

#if SIZE_MAX == UINT_MAX
    #define LEN_PTR 0
#elif SIZE_MAX == ULLONG_MAX
    #define LEN_PTR 2
#endif

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

enum fp_class
{
    FP_CL_NAN = 0,
    FP_CL_INF = 1,
    FP_CL_NORM = 2,
    FP_CL_SUBNORM = 3,
    FP_CL_ZERO = 4,
};

/**
 * @brief to acquire infomation about this fp type and values related to `p`
 *
 * @param p pointer to the float point number to be outputted
 * @param sign sign part of `*p`
 * @param expo exponent part of `*p`
 * @param frac fraction / significand of `*p`
 * @param class fp_class
 * @param bias constant returned as per the document of this type
 * @param fracbits constant returned as per the document of this type
 * @param pow2 preprocessed constant of 2 ** fracbits
 */
void __xfp_binary64(void *p, int *sign, int *expo, u128 *frac, int *class,
                         int *bias, int *fracbits, const big **pow2)
{
    static const big pow2_52 = {
        16, "\x4\x5\x0\x3\x5\x9\x9\x6\x2\x7\x3\x7\x0\x4\x9\x6"};
    uint64_t u64 = *(uint64_t *)p;
    *sign = (int)(u64 >> 63);
    *expo = (int)((u64 >> 52) & 0x7ff);
    *frac = u64 & 0xfffffffffffff;
    if (*expo == 0) {
        *class = !*frac ? FP_CL_ZERO : FP_CL_SUBNORM;
    } else if (*expo == 0x7ff) {
        *class = !*frac ? FP_CL_INF : FP_CL_NAN;
    } else {
        *class = FP_CL_NORM;
    }
    *bias = 1023;
    *fracbits = 52;
    *pow2 = &pow2_52;
}

// fallbacks
#ifndef XFP
    #define XFP __xfp_binary64
#endif

#ifndef LXFP
    #define LXFP(v, ...)  \
        double _v = *(v); \
        XFP(&_v, ##__VA_ARGS__)
#endif

// do dragon4-like algorithm
static int fmt_fp(char **fpbuf, char *prefix, char *suffix, int len, char spec,
                  int flgs, va_list *ap, int *size, int *prec)
{
    // 注意 *size 需要包含 prefix 与 suffix 的长度
    const big *pow2;
    u128 frac;
    int dot, e2;
    int sign, expo, cl;
    int bias, fracbits;
    bool lower = (spec & 32) == 32;
    big_new(bigfrac);
    big_new(result);

    if (len == 0) {
        double v = va_arg(*ap, double);
        XFP(&v, &sign, &expo, &frac, &cl, &bias, &fracbits, &pow2);
    } else {
        long double v = va_arg(*ap, long double);
        LXFP(&v, &sign, &expo, &frac, &cl, &bias, &fracbits, &pow2);
    }
    big_push_u128(&bigfrac, frac);

    if (cl == FP_CL_INF || cl == FP_CL_NAN) {
        static const char *inf_nan[][2] = {
            {"NAN", "nan"},
            {"INF", "inf"},
        };
        const char *str = inf_nan[cl][lower];
        *fpbuf = strdup(str);
        *size = 3;
        *prec = 0;
        goto ret_sign;
    }

    // 0x1.DIGITSp[+-]e: e.g. 0x1.23p+0
    if (spec == 'a' || spec == 'A') {
        prefix[0] = '0';
        prefix[1] = 'X' | (spec & 32);
        prefix[2] = '\0';
        char hid[2], *buf, *hex, exp[32];
        int m, e, hidsz, hexsz, expsz;

        if (cl == FP_CL_ZERO) {
            *fpbuf = strdup(*prec > 0 ? "0." : "0");
            strcpy(suffix, "p+0");
            *size = 4;
            goto ret_sign;
        }
        hexsz = big_tohex(&hex, &bigfrac, lower, prec);
        while (hex[hexsz - 1] == '0')
            hexsz--;
        hex[hexsz] = '\0';
        m = cl != FP_CL_SUBNORM ? 1 : 0;
        hid[0] = m + '0';
        hid[1] = hexsz > 0 || (flgs & SPECIAL) ? '.' : '\0';
        hidsz = hexsz > 0 || (flgs & SPECIAL) ? 2 : 1;

        e = cl != FP_CL_SUBNORM ? expo - bias : 1 - bias;
        {
            int ndigits = 0;
            exp[0] = 'p';
            exp[1] = e >= 0 ? '+' : '-';
            if (e < 0) e = -e;
            for (int x = e; x; x /= 10)
                ndigits++;
            char *p = exp + 2;
            for (int i = 0; i < ndigits; i++, e /= 10)
                p[ndigits - i - 1] = '0' + (e % 10);
            expsz = ndigits + 2;
        }

        *size = hidsz;  // 1.
        *size += hexsz; // DIGITS
        *size += expsz; // p[+-]e
        *fpbuf = malloc(*size + 1);
        if (!*fpbuf) {
            free(hex);
            goto ret_fail;
        }
        strcpy(*fpbuf, hid);         // hidsz
        strcpy(*fpbuf + hidsz, hex); // hexsz
        strcpy(suffix, exp);         // expsz
        free(hex);
        goto ret_sign;
    }

    if (*prec < 0) *prec = 6;
    if (cl == FP_CL_ZERO) {
        // 现在, *prec >= 0
        *fpbuf = strdup(*prec > 0 ? "0." : "0");
        *size = *prec > 0 ? 2 : 1;
        goto ret_sign;
    }
    if (cl != FP_CL_SUBNORM) {
        big_pad(&bigfrac, pow2->len);
        big_add(&bigfrac, pow2);
    }

    e2 = cl != FP_CL_SUBNORM ? expo - bias - fracbits : 1 - bias - fracbits;
    if (e2 < 0) {
        // 因为 除以的是 2 的幂, 这个大整数最终肯定可以除完, 这时候 div 会返回,
        // 也就意味着, 计算结束后 prec 可能仍然没有 "满足", 这时候需要在末尾填上
        // 0. fmp_fp 返回 prec 是还剩余的 0 的个数.
        if (big_a_div_pow2(&bigfrac, -e2, &result, &dot, *prec) < 0)
            goto ret_fail;
        *prec -= result.len - dot;
    } else {
        if (big_a_mul_pow2(&bigfrac, e2, &result) < 0) goto ret_fail;
        // 两个数相乘, 结果 (result) 一定是整数, 这时候 dot 没有设置. dot
        // 应该设置成 整数后面
        dot = result.len;
    }
    // *prec > 0, 在 *fpbuf 输出之后结果需要补 0. 考虑 1.0, *fpbuf = "1", 显然,
    // 如果没有小数点, 这个结果就是错误的, 于是小数点刚好落在末尾 时特殊处理.
    *size = big_tostr(fpbuf, &result, dot);
    if (*prec > 0 && dot == result.len) {
        (*fpbuf)[(*size)++] = '.';
        (*fpbuf)[(*size)] = '\0';
    }
ret_sign:
    big_free(&bigfrac);
    big_free(&result);
    return sign ? -1 : 1;
ret_fail:
    *fpbuf = 0;
    prefix[0] = '\0';
    suffix[0] = '\0';
    *size = 0;
    *prec = 0;
    return 0;
}

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

        int len = 0, prec = -1, width = 0;
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
        char *s, prefix[4], suffix[32], tmp[32], *fptmp = 0;
        prefix[0] = suffix[0] = 0;
        switch (*fmt | 32) {
        case 'c':
            // 实际上, %c / %s 如果遇到 %010c 这样的格式都会被视作无效,
            // 使用空格输出, 这里不做处理
            tmp[0] = (char)va_arg(ap, int);
            tmp[1] = '\0';
            size = 1, s = tmp;
            fmt++;
            break;
        case 's':
            s = (char *)va_arg(ap, char *);
            if (!s) s = "(nil)";
            size = strlen(s);
            fmt++;
            break;
        case 'x':
        case 'o':
        case 'd':
        case 'i':
        case 'u':
            sign = fmt_num(s = tmp, prefix, len, *fmt++, flgs, &ap, &size);
            break;
        case 'p':
            sign = fmt_num(s = tmp, prefix, LEN_PTR, 'x', flgs | SPECIAL, &ap,
                           &size);
            fmt++;
            break;
        case 'g':
        case 'e':
        case 'f':
        case 'a':
            sign = fmt_fp(&fptmp, prefix, suffix, len, *fmt++, flgs, &ap, &size,
                          &prec);
            if (sign == 0) {
                s = "(fmt_fp ENOMEM)";
                size = strlen(s);
                break;
            }
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
        fputs(suffix, f);
        if (pad == ' ' && left)
            while (padsz-- > 0)
                out(pad);
        if (fptmp) free(fptmp);
        n += size;
    }

    return n;
}

