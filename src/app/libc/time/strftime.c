#include <time.h>

#define mkequ(fmt)                           \
    do {                                     \
        int ret = strftime(s, max, fmt, tm); \
        if (ret == 0) return 0;              \
        s += ret, max -= ret;                \
    } while (0);

// using stdio cookie is a good idea, but it costs too much
size_t strftime(char *restrict s, size_t max, const char *restrict format,
                const struct tm *restrict tm)
{
    const char *s0 = s;
    for (char c; (c = *format); format++) {
        int v = -1, w = 0, pad = 0;
        if (c != '%') {
            *s++ = c, max -= 1;
            continue;
        }
        c = *++format;
        switch (c) {
            // %a
            // %A
            // %b
            // %B
            // %c
        case 'C':
            v = 19 + tm->tm_year / 100;
            pad = '0';
            break;
        case 'd':
            v = tm->tm_mday;
            pad = '0';
            w = 2;
            break;
        case 'D':
            mkequ("%m/%d/%y");
            continue;
        case 'e':
            v = tm->tm_mday;
            pad = ' ';
            w = 2;
            break;
        case 'F':
            mkequ("%Y-%m-%d");
            continue;
            // %g
            // %G
            // %h
        case 'H':
            v = tm->tm_hour;
            pad = '0';
            w = 2;
            break;
            // %I
            // %j
        case 'm':
            v = tm->tm_mon;
            pad = '0';
            w = 2;
            break;
        case 'M':
            v = tm->tm_min;
            pad = '0';
            w = 2;
            break;
        case 'n':
            *s++ = '\n';
            continue;
            // %p
            // %r
        case 'R':
            mkequ("%H:%M");
            continue;
        case 'S':
            v = tm->tm_sec;
            pad = '0';
            w = 2;
            break;
        case 't':
            *s++ = '\t';
            continue;
        case 'T':
            mkequ("%H:%M:%S");
            continue;
        case 'u':
            v = tm->tm_wday == 0 ? 7 : tm->tm_wday;
            break;
            // %U
            // %V
            // %w
            // %W
            // %x
            // %X
        case 'y':
            v = tm->tm_year % 100;
            pad = '0';
            w = 2;
            break;
        case 'Y':
            v = 1900 + tm->tm_year;
            w = 4;
            break;
            // %z
            // %Z
        case '%':
            *s++ = '%';
            continue;
        default:
            return 0;
        }

        // convert to string
        {
            int nd = 0;
            for (int x = v; x; x /= 10)
                nd++;
            if (v == 0) nd = 1;
            if (pad) {
                for (int i = w - nd; i > 0; i--) {
                    if (max <= 0) return 0;
                    *s++ = pad, max--;
                }
            }
            if (max < nd) return 0;
            if (v == 0)
                s[0] = '0';
            else
                for (int j = nd - 1; v; j--, v /= 10) {
                    s[j] = '0' + v % 10;
                }
            s += nd;
        }
    }
    if (max <= 0) return 0;
    *s = '\0';
    return (size_t)(s - s0);
}
