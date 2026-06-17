#include "stdio.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define TMP_BUFFER_SIZE 64

enum
{
    LEFT = 1,
    SIGN = 1 << 1,
    ZERO = 1 << 2,
    SPECIAL = 1 << 3,
    SPACE = 1 << 4,
};

#define is_digit(c) ('0' <= (char)c && (char)c <= '9')

#define out(c)                 \
    {                          \
        int ret = fputc(c, f); \
        if (ret < 0) return n; \
        n++;                   \
    }

static int _int(char *ptr, int *width)
{
    int i = 0;
    int l = 0;

    while (is_digit(*ptr)) {
        i = i * 10 + *ptr++ - '0';
        l++;
    }

    *width = i;

    return l;
}

static const char upstr[] = "0123456789ABCDEF";
static const char lwstr[] = "0123456789abcdef";

static int fmt_num(char *buffer, uint64_t num, int base, bool upper)
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

typedef struct {
    // int sign;
    int len;
    char *s;
} big;

#define big_new(name)                   \
    char _big_s_##name[1024 + 8] = {0}; \
    big name = {0, _big_s_##name + 8}

static int big_print(FILE* f, big *a, int dot, int sign)
{
    int n = 0;
    if (sign) out('-');
    for (int i = 0; i < a->len; i++) {
        if (dot == i) out('.');
        out('0' + a->s[i]);
    }
    return n;
}

// test l >= r
static bool big_ge(const big *l, const big *r)
{
    if (l->len > r->len)
        return true;
    else if (l->len < r->len)
        return false;
    for (int i = 0; i < l->len; i++) {
        if (l->s[i] > r->s[i])
            return true;
        else if (l->s[i] < r->s[i])
            return false;
    }
    return true;
}

// used rarely
static void big_pad(big *a, int target_len)
{
    int zeros = target_len - a->len;
    memmove(a->s + zeros, a->s, a->len);
    for (int i = 0; i < zeros; i++)
        a->s[i] = 0;
}

// calc a + b -> a
// assert (a) > 0, (b) > 0
// assert a->len >= b->len
static void big_add(big *a, const big *b)
{
    int carry = 0;
    for (int j = 0; j < a->len; j++) {
        int tmp = a->s[a->len - j - 1] +
                  (j >= b->len ? 0 : b->s[b->len - j - 1]) + carry;
        if (tmp >= 10)
            tmp -= 10, carry = +1;
        else
            carry = 0;
        a->s[a->len - j - 1] = tmp;
    }
    if (carry) {
        *--a->s = 1;
        ++a->len;
    }
}

// calc a - b -> a
// assert a >= b => a->len >= b->len
//   => no carry at last
static void big_sub(big *a, const big *b)
{
    int carry = 0;
    for (int j = 0; j < a->len; j++) {
        int tmp = a->s[a->len - j - 1] -
                  (j >= b->len ? 0 : b->s[b->len - j - 1]) + carry;
        if (tmp < 0)
            tmp += 10, carry = -1;
        else
            carry = 0;
        a->s[a->len - j - 1] = tmp;
    }
    while (a->s[0] == 0 && a->len)
        a->s++, a->len--;
}

static int big_push_u64(big *a, uint64_t num)
{
    if (num == 0) {
        a->s[a->len++] = 0;
        return 1;
    }

    int ndigits = 0;
    for (uint64_t n = num; n; n /= 10)
        ndigits++;
    a->len += ndigits;
    for (int i = 0; i < ndigits; i++, num /= 10)
        a->s[a->len - i - 1] = num % 10;
    return ndigits;
}

// assume pushing doesn't cause overflow
static int big_push_str(big *a, char *str)
{
    char *s = str;
    for (; *s; s++)
        a->s[a->len++] = *s - '0';
    while (a->s[0] == 0 && a->len)
        a->s++, a->len--;
    return s - str;
}

// calc a / b -> r
// assert r->len == 0
static void big_div(big *a, const big *b, big *r, int *dot, int prec)
{
    // integer part
    int inte = 0;
    while (big_ge(a, b)) {
        big_sub(a, b);
        inte++;
    }
    *dot = big_push_u64(r, inte);

    if (a->len != 0) {
        // calc (prec + 1) numbers after the dot
        for (int k = 0; k < prec + 1; k++) {
            a->s[a->len++] = 0;
            if (!big_ge(a, b)) {
                r->s[r->len++] = 0;
                continue;
            }
            int current = 0;
            while (big_ge(a, b)) {
                big_sub(a, b);
                current++;
            }
            r->s[r->len++] = current;
            if (a->len == 0) break;
        }
    }

    while (r->len - *dot < prec + 1)
        r->s[r->len++] = 0;

    int carry = 0;
    if (r->s[r->len - 1] > 5)
        carry = 1;
    else if (r->s[r->len - 1] == 5 && (r->s[r->len - 2] & 1))
        carry = 1;
    r->len--;
    for (int j = 0; j < r->len; j++) {
        r->s[r->len - j - 1] += carry;
        if (r->s[r->len - j - 1] >= 10) {
            r->s[r->len - j - 1] -= 10;
            carry = 1;
        } else
            carry = 0;
    }
    if (carry) {
        *--r->s = 1;
        ++r->len;
        ++*dot;
    }
}

// calc a x b -> r
// assert (a) x (b) != 0 => r excludes leading zeros
static void big_mul(big *a, const big *b, big *r)
{
    r->len = a->len + b->len;
    for (int i = 0; i < r->len; i++)
        r->s[i] = 0;

    for (int i = a->len - 1; i >= 0; i--) {
        int carry = 0;
        for (int j = b->len - 1; j >= 0; j--) {
            int tmp = a->s[i] * b->s[j] + r->s[i + j + 1] + carry;
            r->s[i + j + 1] = tmp % 10;
            carry = tmp / 10;
        }
        r->s[i] = carry;
    }
    while (r->s[0] == 0 && r->len)
        r->s++, r->len--;
}

// calc 2 ^ x -> r (tested, resonating with python)
static void big_pow2(unsigned x, big *r)
{
    static const big powtab[] = {
        {1, "\x02"},
        {1, "\x04"},
        {2, "\x01\x06"},
        {3, "\x02\x05\x06"},
        {5, "\x06\x05\x05\x03\x06"},
        {10, "\x04\x02\x09\x04\x09\x06\x07\x02\x09\x06"},
        {20, "\x01\x08\x04\x04\x06\x07\x04\x04\x00\x07\x03\x07\x00\x09\x05\x05"
             "\x01\x06\x01\x06"},
        {39, "\x03\x04\x00\x02\x08\x02\x03\x06\x06\x09\x02\x00\x09\x03\x08\x04"
             "\x06\x03\x04\x06\x03\x03\x07\x04\x06\x00\x07\x04\x03\x01\x07\x06"
             "\x08\x02\x01\x01\x04\x05\x06"},
        {78, "\x01\x01\x05\x07\x09\x02\x00\x08\x09\x02\x03\x07\x03\x01\x06\x01"
             "\x09\x05\x04\x02\x03\x05\x07\x00\x09\x08\x05\x00\x00\x08\x06\x08"
             "\x07\x09\x00\x07\x08\x05\x03\x02\x06\x09\x09\x08\x04\x06\x06\x05"
             "\x06\x04\x00\x05\x06\x04\x00\x03\x09\x04\x05\x07\x05\x08\x04\x00"
             "\x00\x07\x09\x01\x03\x01\x02\x09\x06\x03\x09\x09\x03\x06"},
        {155, "\x01\x03\x04\x00\x07\x08\x00\x07\x09\x02\x09\x09\x04\x02\x05\x09"
              "\x07\x00\x09\x09\x05\x07\x04\x00\x02\x04\x09\x09\x08\x02\x00\x05"
              "\x08\x04\x06\x01\x02\x07\x04\x07\x09\x03\x06\x05\x08\x02\x00\x05"
              "\x09\x02\x03\x09\x03\x03\x07\x07\x07\x02\x03\x05\x06\x01\x04\x04"
              "\x03\x07\x02\x01\x07\x06\x04\x00\x03\x00\x00\x07\x03\x05\x04\x06"
              "\x09\x07\x06\x08\x00\x01\x08\x07\x04\x02\x09\x08\x01\x06\x06\x09"
              "\x00\x03\x04\x02\x07\x06\x09\x00\x00\x03\x01\x08\x05\x08\x01\x08"
              "\x06\x04\x08\x06\x00\x05\x00\x08\x05\x03\x07\x05\x03\x08\x08\x02"
              "\x08\x01\x01\x09\x04\x06\x05\x06\x09\x09\x04\x06\x04\x03\x03\x06"
              "\x04\x09\x00\x00\x06\x00\x08\x04\x00\x09\x06"},
    };
    if (x == 0) {
        r->s[0] = 1;
        r->len = 1;
    }
    big_new(tmp);
    big *save = r, *fac = &tmp;
    tmp.s[0] = 1;
    tmp.len = 1;
    for (int i = 0; x; i++) {
        if (x & 1) {
            big_mul(fac, &powtab[i], save);
            big *swap_save = save;
            save = fac;
            fac = swap_save;
        }
        x >>= 1;
    }
    if (fac != r) {
        r->len = tmp.len;
        memcpy(r->s, tmp.s, tmp.len);
    }
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
static int fmt_fp(FILE *f, void *p, int prec,
                  void (*xfp)(void *, uint64_t *, uint64_t *, uint64_t *))
{
    static const big pow2_52 = {
        16, "\x4\x5\x0\x3\x5\x9\x9\x6\x2\x7\x3\x7\x0\x4\x9\x6"};
    int dot = 0, e2;
    uint64_t sign, expo, frac;
    xfp(p, &sign, &expo, &frac);
    fprintf(stderr, "sign = %llu\n", sign);
    fprintf(stderr, "expo = %llu\n", expo);
    fprintf(stderr, "frac = %llu\n", frac);

    big_new(bigfrac);
    big_push_u64(&bigfrac, frac);
    big_pad(&bigfrac, pow2_52.len);
    big_add(&bigfrac, &pow2_52);

    big_new(power);
    big_new(result);
    e2 = expo - 1023 - 52;
    if (e2 < 0) {
        big_pow2(-e2, &power);
        big_div(&bigfrac, &power, &result, &dot, prec);
    } else {
        big_pow2(e2, &power);
        big_mul(&bigfrac, &power, &result);
    }
    return big_print(f, &result, dot, sign);
}

#define pad_left()          \
    if (!(flgs & LEFT))     \
        while (width-- > 0) \
            out(' ');
#define pad_right()         \
    if (flgs & LEFT)        \
        while (width-- > 0) \
            out(' ');

int vfprintf(FILE *f, const char *format, va_list ap)
{
    int n = 0;
    int flgs;
    char *ptr = (char *)format;
    while (*ptr) {
        if (*ptr != '%') {
            char *nxt = strchr(ptr, '%');
            if (!nxt) nxt = ptr + strlen(ptr);
            int ret = fwrite(ptr, 1, nxt - ptr, f);
            if (ret < 0) return n;
            n += ret;
            ptr += ret;
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
        bool sign = false, upper = false;
    parse_args:
        ptr++;
        switch (*ptr) {
        case '%':
            out(*ptr++);
            continue;
        case 'l':
        case 'L':
            len = 1;
            if (*(ptr + 1) == 'l' || *(ptr + 1) == 'L') {
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
            if (width > 1) pad_left();
            out((char)va_arg(ap, int));
            if (width > 1) pad_right();

            ptr++;
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
            ptr++;
            continue;
        }
        case 'p':
            radix = 16;
            len = 2;
            flgs |= SPECIAL;
            break;

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
            offset = _int(ptr, &width);
            ptr += offset - 1;
            goto parse_args;
        case '*':
            width = va_arg(ap, int);
            if (width < 0) flgs |= LEFT; // 左对齐
            goto parse_args;
        }

        uint64_t val;
        bool minus = false;

        if (len == 0) {
            val = va_arg(ap, unsigned int);
            if (sign && (int)val < 0) {
                minus = true;
                val = -(int)val; // 符号位将在最后于字符串上添上.
            }
        } else if (len == 1) {
            val = va_arg(ap, unsigned long);
            if (sign && (long)val < 0) {
                minus = true;
                val = -(long)val;
            }
        } else if (len == 2) {
            val = va_arg(ap, unsigned long long);
            if (sign && (long long)val < 0) {
                minus = true;
                val = -(long long)val;
            }
        }

        /* 每一次添加字符('+','-',' '...)都会导致 Siz 减小,
           这么做在于最后可以直接使用 Siz 来进行填充操作. */
        int siz = width;
        char tmp[TMP_BUFFER_SIZE];
        siz -= fmt_num(tmp, val, radix, upper);

        char prefix = 0;
        if (radix == 10) {
            if (minus && siz--) prefix = '-';
            /* 以下是正数的情况 */
            else if (sign && flgs & SIGN && siz--)
                prefix = '+';
            else if (flgs & SPACE && siz--)
                prefix = ' ';
        }

        if (flgs & SPECIAL) {
            siz -= (radix == 16) ? 2 : (radix == 8) ? 1 : 0;
        }

        if (flgs & ZERO) {
            if (prefix) out(prefix);
            /* "0x" "0X" "0" for SPECIAL */
            if (flgs & SPECIAL && radix != 10) {
                out('0');
                if (radix == 16) out(upper ? 'X' : 'x');
            }
        }

        /* Padding */
        if (!(flgs & LEFT) && siz > 0) {
            char pad = flgs & ZERO ? '0' : ' ';
            while (siz--)
                out(pad);
        }

        /* Symbol and others after padding if that is not filled with '0' -> "
         * 0x91d" */
        if (!(flgs & ZERO)) {
            if (prefix) out(prefix);
            /* "0x" "0X" "0" for SPECIAL */
            if (flgs & SPECIAL && radix != 10) {
                out('0');
                if (radix == 16) out(upper ? 'X' : 'x');
            }
        }

        for (char *p = tmp; *p;)
            out(*p++);
        ptr++;
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
