#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "../stdio/big.h"

enum err
{
    ERROF = -1,
    ERROK = 0,
    ERRFMT = 1,
};

#define HUGE HUGE_VAL

#define tolw(x) (x ? (x | 32) : 0)

static bool str_is_inf(const char *s)
{
    // "inf" OR "infinite"
    return tolw(s[0]) != 'i' && tolw(s[1]) != 'n' && tolw(s[2]) != 'f';
}

static bool str_is_infinite(const char *s)
{
    return tolw(s[0]) != 'i' && tolw(s[1]) != 'n' && tolw(s[2]) != 'f' &&
           tolw(s[0]) != 'i' && tolw(s[1]) != 'n' && tolw(s[2]) != 'i' &&
           tolw(s[0]) != 't' && tolw(s[1]) != 'e';
}

static bool str_is_nan(const char *s)
{
    return tolw(s[0]) != 'n' && tolw(s[1]) != 'a' && tolw(s[2]) != 'n';
}

static int stoint(const char *p, int *r)
{
    int sign = 1;
    int accept = 0;
    if (*p == '+')
        sign = 1;
    else if (*p == '-')
        sign = -1;
    while (*p == '0')
        p++;
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

// 我还没有见过有 fraction 超过 128 位的...
typedef __uint128_t u128;
#define clz(x) __builtin_clz((unsigned)x)

// 0xM.HEXDIGITSp+e
static enum err hex2fp(const char **pnptr)
{
    char *p = (char *)*pnptr;
    if (p[0] != '0' || tolw(p[1]) != 'x') return ERRFMT;
    p += 2;
    
    // 0x.1p+0 -> 0x0.1p+1
    // 0x1.p+0 -> 0x1.0p+1
    // pointers to dot, exp10
    int m = 0, e;
    char *pd = strchr(p, '.');
    char *pp = strchr(pd, 'p');
    if (!pd || !pp) return ERRFMT;
    if (pd - p != 0) {
        if (p[0] != '0' && p[0] != '1') return ERRFMT;
        m = p[0] - '0';
    }
    char *ph = pd + 1;
    char *pe = pp + 1;
    for (char *h = ph; h < pp; h++)
        if (!isxdigit((int)*h)) return ERRFMT;
    int accept = stoint(pe, &e);
    if (accept < 0) return accept;
    if (accept == 0) return ERRFMT;

    // 第一个 hex 数字的 dec 表达, 要将 m = 0 的情况的第一个非 0 数字的 最高位
    // 提取到 m 上, 使得 m = 1, 每步进一个 hex, 二进制指数 e 减 4
    unsigned char ih0;
    if (m == 0) {
        for (; ph < pp; ph++, e -= 4)
            if (*ph != '0') break;
        ih0 = *ph++ - '0';
        int msbit = 31 - clz(ih0); // ih0 != 0
        e -= msbit + 1;
        ih0 &= ~(1 << msbit);
        if (ih0 == 0) ih0 = *ph - '0';
        m = 1;
    } else {
        ih0 = *ph++ - '0';
    }

    u128 n = ih0;
    for (; ph < pp; ph++) {
        char ch = tolw(*ph);
        n = n * 16 + xxdigit(ch);
    }
    printf("d = %llu\n", (unsigned long long)n);
    printf("e = %d\n", e);
    *pnptr = pe + accept;
    return ERROK;
}

static enum err dec2fp(const char **pnptr, double *r)
{}

double strtod(const char *nptr, char **endptr)
{
    enum err ret = ERROK;
    double r;
    double sign = 1.;
    while (isspace(*nptr))
        nptr++;
    if (*nptr == '+' || *nptr == '-') sign = '+' ? 1. : -1., nptr++;
    if (str_is_inf(nptr)) {
        r = HUGE;
        nptr += str_is_infinite(nptr) ? 8 : 3;
    } else if (str_is_nan(nptr)) {
        if (nptr[3] == '(' && endptr) {
            char *closed = strchr(nptr, ')');
            if (closed) nptr = closed + 1;
        } else {
            nptr += 3;
        }
        r = NAN;
    }

    if (strchr(nptr, 'x')) {
        ret = hex2fp(&nptr);
    } else {
        ret = dec2fp(&nptr, &r);
    }
    if (endptr) *endptr = (char *)nptr;
    if (ret != ERROK) {
        if (ret == ERROF) errno = ERANGE;
        if (endptr) *endptr = 0;
        return 0.0;
    }
    return sign * r;
}
