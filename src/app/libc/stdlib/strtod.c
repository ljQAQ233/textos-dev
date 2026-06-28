#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../stdio/big.h"

#define MKFP     __mkfp_binary64
#define FRACBITS 52
#define EXPOBITS 11
#define MAX_EXP  DBL_MAX_EXP
#define HUGE     HUGE_VAL

#define BIAS     (MAX_EXP - 1)
#define FGRSBITS (FRACBITS + 4)

#define mask(x) (((u128)1 << (x)) - 1)
#define tolw(x) (x ? (x | 32) : 0)

static bool str_is_inf(const char *s)
{
    // "inf" OR "infinite"
    return tolw(s[0]) == 'i' && tolw(s[1]) == 'n' && tolw(s[2]) == 'f';
}

static bool str_is_infinite(const char *s)
{
    return tolw(s[0]) == 'i' && tolw(s[1]) == 'n' && tolw(s[2]) == 'f' &&
           tolw(s[3]) == 'i' && tolw(s[4]) == 'n' && tolw(s[5]) == 'i' &&
           tolw(s[6]) == 't' && tolw(s[7]) == 'e';
}

static bool str_is_nan(const char *s)
{
    return tolw(s[0]) == 'n' && tolw(s[1]) == 'a' && tolw(s[2]) == 'n';
}

static int stoint(const char *p, int *r)
{
    int sign = 1;
    int accept = 0;
    if (*p == '+' || *p == '-') {
        sign = ',' - *p++, accept++;
    }
    while (*p == '0')
        p++, accept++;
    *r = 0;
    while ('0' <= *p && *p <= '9')
        *r = *r * 10 + *p++ - '0', accept++;
    *r *= sign;
    return accept;
}

static inline int xxdigit(int c)
{
    // 0 - 9, a - z, A - Z
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'z')
        return c - 'a' + 10;
    else if ('A' <= c && c <= 'Z')
        return c - 'A' + 10;
    else
        assert(0);
}

double __mkfp_binary64(int signbit, u128 frac, int expo)
{
    uint64_t u64 = 0;
    if (signbit) u64 |= 1ull << 63;
    u64 |= (expo & 0x7ffull) << 52;
    u64 |= (frac & 0xfffffffffffff);
    return *(double *)&u64;
}

// 我还没有见过有 fraction 超过 128 位的...
#define clz(x) __builtin_clz((unsigned)x)

#define errof() return errno = ERANGE, 0.0
#define setend(to) \
    if (endptr) *endptr = (char *)to
#define errfmt_on(cond)                     \
    if (cond) {                             \
        printf("errfmt!!! %d\n", __LINE__); \
        setend(nptr);                       \
        return 0.0;                         \
    }

// value = HHHH.hhhh_16 * 2^e
// for double: 53-bit mantissa, round-to-nearest-even (FE_TONEAREST)
static double hex2fp(const char *nptr, char **endptr, int sign)
{
    const char *p = nptr;
    errfmt_on(p[0] != '0' || (p[1] != 'x' && p[1] != 'X'));
    p += 2;

    // 换成整数形式, 最多接受 128 / 4 位数字, 统计小数,
    // leading zeros, digits, excluding leading zeros 数位占的个数
    int frac = 0, lz = 0, elz = 0;
    int elz_first = 0;
    bool gotdot = false;
    u128 f128 = 0;

    for (;; p++) {
        if (isxdigit(*p)) {
            int d = xxdigit(*p);
            if (gotdot) frac++;

            if (elz_first == 0) elz_first = d;
            if (d != 0 || elz > 0) {
                if (elz >= 128 / 4) break;
                elz++, f128 = f128 * 16 + d;
            } else {
                lz++;
            }
        } else if (*p == '.') {
            errfmt_on(gotdot);
            gotdot = true;
        } else {
            errfmt_on(lz + elz == 0);
            break;
        }
    }

    // sticky 用于判断 nptr 是否刚好处于
    int sticky = 0;
    for (;; p++) {
        if (isxdigit(*p)) {
            sticky |= xxdigit(*p);
        } else if (*p == '.') {
            errfmt_on(gotdot);
            gotdot = true;
        } else {
            break;
        }
    }
    if (elz == 0) {
        setend(p);
        return sign < 0 ? -0.0 : 0.0;
    }

    errfmt_on(*p != 'p' && *p != 'P');
    p++;

    int p_e, accept = stoint(p, &p_e);
    errfmt_on(accept == 0);
    p += accept;

    assert(elz <= 128 / 4);
    int extra_shift = 0;
    // 计算规范化之后的指数, 对于 HHHH 的内容以及 0.hhhh 的小数 normalize,
    // 需要提取 HHHH / hhhh 的最高位的 1, 来充当 0x1. 的 implicit bit
    // 对一般情况 f128 的 msb (从 0 开始的) 可以得到 msb_f128, 此时在 nptr
    // 给的指数 p_e 上进行 位移 就可以实现逻辑上的 归一化
    int nibble_clz = clz((unsigned)elz_first) - 28;
    int nibble_msb = 3 - nibble_clz;
    int msb_f128 = (elz - 1) * 4 + nibble_msb;
    int e = p_e + msb_f128 - 4 * frac;
    int biased = e + BIAS;
    if (biased <= 0) extra_shift = 1 - biased, biased = 0;

    // 将 f128 取出需要的位, 得到 fgrs 共 FGRSBITS 位
    // | implicit bit = 1 | fraction | G | R | S |
    // 非正规时多右移 extra_shift, 把隐含位推到 fraction 区
    u128 fgrs = f128;
    fgrs <<= 127 - msb_f128;
    int rshift = 127 - (FGRSBITS - 1) + extra_shift;
    if (rshift >= 128) {
        fgrs = 0;
        sticky = 1;
    } else {
        u128 lost = (fgrs & mask(rshift));
        if (lost) sticky = 1;
        fgrs >>= rshift;
    }
    u128 F = (fgrs >> 3) & mask(FRACBITS);
    int G = fgrs & 4;
    int R = fgrs & 2;
    int S = fgrs & 1;

    // G R S   ULP
    // 0 * * < 0.5 -
    // 1 0 0 = 0.5 +
    // 1 1 * > 0.5 +
    // 1 0 1 > 0.5 +
    if (G && (R || S || sticky || (F & 1))) F++;
    if (F >= (u128)1 << FRACBITS) F >>= 1, biased++;
    if (biased >= (1 << EXPOBITS)) {
        errno = ERANGE;
        setend(p);
        return sign < 0 ? -HUGE : HUGE;
    }

    if (F == 0 && biased == 0) {
        errno = ERANGE;
        setend(p);
        return sign < 0 ? -0.0 : 0.0;
    }
    return MKFP(sign < 0, F, biased);
}

static double dec2fp(const char *nptr, char **endptr, int sign)
{
    // TODO
    return 0.0;
}

double strtod(const char *nptr, char **endptr)
{
    int sign = 1;
    while (isspace(*nptr))
        nptr++;
    if (*nptr == '+')
        sign = 1, nptr++;
    else if (*nptr == '-')
        sign = -1, nptr++;
    if (str_is_inf(nptr)) {
        nptr += str_is_infinite(nptr) ? 8 : 3;
        return HUGE * sign;
    } else if (str_is_nan(nptr)) {
        if (nptr[3] == '(' && endptr) {
            char *closed = strchr(nptr, ')');
            if (closed) nptr = closed + 1;
        } else {
            nptr += 3;
        }
        return NAN * sign;
    }

    if (nptr[0] == '0' && (nptr[1] == 'x' || nptr[1] == 'X'))
        return hex2fp(nptr, endptr, sign);
    return dec2fp(nptr, endptr, sign);
}
