#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

static inline int extract(int c, int base, long *v)
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

long strtol(const char *restrict nptr, char **restrict endptr, int base)
{
    errno = 0;
    const char *end = nptr;
    long ret = 0;
    long sign = 1;
    if ((base < 0) || (base == 1) || (base > 36)) {
        if (endptr != NULL) {
            *endptr = (char *)end;
        }
        return 0;
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
    /* calc cutoff (1 < base <= 36) */
    long cutoff = (sign > 0) ? LONG_MAX / base : -(LONG_MIN / base);
    long cutlim = (sign > 0) ? LONG_MAX % base : -(LONG_MIN % base);
    /* digits follow */
    long digit;
    while (extract((unsigned char)*nptr, base, &digit)) {
        if ((ret > cutoff) || (ret == cutoff && digit > cutlim)) {
            if (sign < 0)
                ret = LONG_MIN;
            else ret = LONG_MAX;
            sign = 1;
            errno = ERANGE;
            while (extract(*nptr, base, &digit))
                nptr++;
            break;
        }
        ret = ret * base + digit;
        end = ++nptr;
    }

    if (endptr) {
        *endptr = (char *)end;
    }
    return sign * ret;
}
