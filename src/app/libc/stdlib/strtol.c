#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

// for strtol
#ifndef NAME
typedef long TYPE;
#define NAME strtol
#define MAX LONG_MAX
#define MIN LONG_MIN
#define SIGNED 1
#endif

static inline int extract(int c, int base, TYPE *v)
{
    // 0 - 9, a - z, A - Z
    if ('0' <= c && c <= '9')
        *v = c - '0';
    else if ('a' <= c && c <= 'z')
        *v = c - 'a' + 10;
    else if ('A' <= c && c <= 'Z')
        *v = c - 'A' + 10;
    else
        return 0;
    return *v < base;
}

TYPE NAME(const char *restrict nptr, char **restrict endptr, int base)
{
    errno = 0;
    const char *end = nptr;
    TYPE ret = 0;
    TYPE sign = 1;
    if ((base < 0) || (base == 1) || (base > 36)) {
        goto l_exit;
    }
    while (isspace((unsigned char)*nptr))
        nptr++;
    if (*nptr == '-' || *nptr == '+') {
        sign = *nptr == '-' ? -1 : 1;
        nptr += 1;
    }

    if (base == 0) {
        if (nptr[0] == '0') {
            if (tolower(nptr[1]) == 'x')
                base = 16, nptr += 2;
            else
                base = 8, nptr += 1;
        } else
            base = 10; /* by default */
    }
    if (sign < 0 && !SIGNED) {
        ret = MIN;
        errno = ERANGE;
        goto l_exit;
    }
    /* calc cutoff (1 < base <= 36) */
    /* for unsigned int, sign > 0 */
    TYPE cutoff = (sign > 0) ? MAX / base : -(MIN / base);
    TYPE cutlim = (sign > 0) ? MAX % base : -(MIN % base);
    /* digits follow */
    TYPE digit;
    while (extract((unsigned char)*nptr, base, &digit)) {
        if ((ret > cutoff) || (ret == cutoff && digit > cutlim)) {
            if (sign < 0)
                ret = MIN;
            else ret = MAX;
            sign = 1;
            errno = ERANGE;
            while (extract((unsigned char)*nptr, base, &digit))
                nptr++;
            goto l_exit;
        }
        ret = ret * base + digit;
        end = ++nptr;
    }

l_exit:
    if (endptr) {
        *endptr = (char *)end;
    }
    return sign * ret;
}
