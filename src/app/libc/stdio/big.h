// this file belongs to vfprintf.c (fmt_fp part)
#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef __uint128_t u128;

#define use_retval __attribute__((warn_unused_result))

typedef struct
{
    int len;
    char *s;
    void *mem;
} big;

#define big_new(name)                   \
    char _big_s_##name[1024 + 8] = {0}; \
    big name = {0, _big_s_##name + 8, 0}

#define big_nil(name) big name = {0, 0, 0}

#define BIGHOLE 8
#define BIGNIL

static bool big_is_zero(const big *a)
{
    return !a->len;
}

static void big_copy(const big *from, big *to)
{
    to->len = from->len;
    memcpy(to->s, from->s, from->len);
}

static void big_free(big *a)
{
    if (a->mem) {
        free(a->mem);
        a->len = 0;
        a->s = NULL;
        a->mem = NULL;
    }
}

use_retval static int big_alloc(BIGNIL big *a, unsigned len)
{
    assert(a->mem == NULL);
    a->mem = malloc(len + BIGHOLE);
    if (!a->mem) return -1;
    a->len = len;
    a->s = a->mem + BIGHOLE;
    return 0;
}

use_retval static int big_extend(big *a, unsigned addition)
{
    void *mem = malloc(a->len + addition + BIGHOLE);
    void *s = mem + BIGHOLE;
    memcpy(s, a->s, a->len);
    a->mem = mem;
    a->s = s;
    return 0;
}

static void big_trim_zero(big *a)
{
    int z = 0;
    while (!a->s[z] && z < a->len)
        z++;
    memmove(a->s, a->s + z, a->len - z);
    a->len -= z;
}

use_retval static int big_mul_small(const big *a, int d, BIGNIL big *r)
{
    if (big_alloc(r, a->len + 1) < 0) return -1;
    int carry = 0;
    for (int i = a->len - 1; i >= 0; i--) {
        int tmp = a->s[i] * d + carry;
        r->s[i + 1] = tmp % 10;
        carry = tmp / 10;
    }
    r->s[0] = carry;
    if (!carry) {
        memmove(r->s, r->s + 1, a->len);
        r->len--;
    }
    return 0;
}

use_retval static int big_tostr(char **buf, const big *a, int dot)
{
    *buf = malloc(a->len + 4);
    if (!*buf) return -1;
    char *p = *buf;
    if (dot == 0) *p++ = '0';
    for (int i = 0; i < a->len; i++) {
        if (dot == i) *p++ = '.';
        *p++ = ('0' + a->s[i]);
    }
    *p++ = '\0';
    return p - *buf - 1;
}

use_retval static int big_tou128(u128 *u, int *stikcy, big *a, int *msb)
{
    *u = 0;
    *stikcy = 0;
    *msb = 0;
    if (big_is_zero(a)) return 0;

    int len = a->len;
    char *num = malloc(len);
    if (!num) return -1;
    memcpy(num, a->s, len);

    int max_bytes = len * 2;
    uint8_t *bits = malloc(max_bytes);
    if (!bits) {
        free(num);
        return -1;
    }
    memset(bits, 0, max_bytes);

    int total = 0;
    int start = 0;
    for (;;) {
        int rem = 0;
        for (int i = start; i < len; i++) {
            int cur = rem * 10 + num[i];
            num[i] = cur >> 1;
            rem = cur & 1;
        }
        if (rem) bits[total / 8] |= (uint8_t)(128 >> (total % 8));
        total++;
        while (start < len && !num[start])
            start++;
        if (start == len) break;
    }
    free(num);

    *msb = total - 1;

    int n = total < 128 ? total : 128;
    for (int i = 0; i < n; i++) {
        int bi = total <= 128 ? i : total - 128 + i;
        if (bits[bi / 8] & (128 >> (bi % 8)))
            *u |= (u128)1 << i;
    }
    for (int i = 0; total > 128 && i < total - 128; i++) {
        if (bits[i / 8] & (128 >> (i % 8))) {
            *stikcy = 1;
            break;
        }
    }
    free(bits);
    return 0;
}

use_retval static int big_tohex(char **buf, big *a, bool lower, int hexcnt,
                                int *prec, int *erupt)
{
    static const char upstr[] = "0123456789ABCDEF";
    static const char lwstr[] = "0123456789abcdef";
    const char *letters = !lower ? upstr : lwstr;
    int len = a->len;
    if (len == 0) return 0;

    char *hex = malloc(len + 1 + 1);
    if (!hex) return -1;
    int hexsz = 0;

    char *num = malloc(len);
    if (!num) {
        free(hex);
        return -1;
    }
    memcpy(num, a->s, len);
    // big int / small int 的长除法, 来模拟短除法. 每运行一轮, num 都除以 16
    for (int start = 0; start < len;) {
        int rem = 0;
        for (int i = start; i < len; i++) {
            int cur = rem * 10 + num[i];
            num[i] = cur / 16;
            rem = cur % 16;
        }
        hex[hexsz++] = rem;
        while (start < len && num[start] == 0)
            start++;
    }
    free(num);

    // 翻转
    for (int i = 0; i < hexsz / 2; i++) {
        char tmp = hex[i];
        hex[i] = hex[hexsz - 1 - i];
        hex[hexsz - 1 - i] = tmp;
    }
    // 向下对齐
    if (hexcnt > hexsz) {
        int zeropad = hexcnt - hexsz;
        hex = realloc(hex, hexcnt);
        memmove(hex + zeropad, hex, hexsz);
        hexsz += zeropad;
        while (--zeropad >= 0)
            hex[zeropad] = 0;
    }

    // 约取
    *erupt = 0;
    if (*prec < 0) {
        while (hex[hexsz - 1] == 0)
            hexsz--;
        *prec = 0;
    } else if (hexsz > *prec) {
        int carry = 0;
        if (hex[*prec] > 8 || (hex[*prec] == 8 && (hex[*prec - 1] & 1)))
            carry = 1;
        for (int i = *prec - 1; i >= 0; i--) {
            hex[i] += carry;
            if (hex[i] >= 16) {
                hex[i] -= 16;
                carry = 1;
            } else {
                carry = 0;
            }
        }
        hexsz = *prec;
        *prec = 0;
        *erupt = carry;
    } else {
        *prec -= hexsz;
    }

    for (int i = 0; i < hexsz; i++)
        hex[i] = letters[hex[i]];
    *buf = hex;
    hex[hexsz] = '\0';
    return hexsz;
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
    a->len += zeros;
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
    assert(b->len != 0);
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
    big_trim_zero(a);
}

static int big_push_u64(big *a, uint64_t num)
{
    if (num == 0) return 0;

    int ndigits = 0;
    for (uint64_t n = num; n; n /= 10)
        ndigits++;
    a->len += ndigits;
    for (int i = 0; i < ndigits; i++, num /= 10)
        a->s[a->len - i - 1] = num % 10;
    return ndigits;
}

static int big_push_u128(big *a, u128 num)
{
    if (num == 0) return 0;

    int ndigits = 0;
    for (u128 n = num; n; n /= 10)
        ndigits++;
    a->len += ndigits;
    for (int i = 0; i < ndigits; i++, num /= 10)
        a->s[a->len - i - 1] = num % 10;
    return ndigits;
}

static int big_push_str(big *a, const char *s, int len)
{
    // only this case need triming
    bool trim = !a->len;
    for (int i = 0; i < len; i++)
        a->s[a->len++] = *s++ - '0';
    if (trim) big_trim_zero(a);
    return len;
}

// calc a x b -> r
// assert (a) x (b) != 0
use_retval static int big_mul(const big *a, const big *b, BIGNIL big *r)
{
    if (big_alloc(r, a->len + b->len) < 0) return -1;
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
    big_trim_zero(r);
    return 0;
}

// square
use_retval static int big_mul_self(big *a)
{
    big_nil(r);
    if (big_mul(a, a, &r) < 0) return -1;
    big_free(a);
    a->len = r.len;
    a->s = r.s;
    a->mem = r.mem;
    return 0;
}

// calc 2 ^ x -> r (tested, resonating with python)
use_retval static int big_pow2(unsigned x, BIGNIL big *r)
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
        {309,
         "\x01\x07\x09\x07\x06\x09\x03\x01\x03\x04\x08\x06\x02\x03\x01\x05\x09"
         "\x00\x07\x07\x02\x09\x03\x00\x05\x01\x09\x00\x07\x08\x09\x00\x02\x04"
         "\x07\x03\x03\x06\x01\x07\x09\x07\x06\x09\x07\x08\x09\x04\x02\x03\x00"
         "\x06\x05\x07\x02\x07\x03\x04\x03\x00\x00\x08\x01\x01\x05\x07\x07\x03"
         "\x02\x06\x07\x05\x08\x00\x05\x05\x00\x00\x09\x06\x03\x01\x03\x02\x07"
         "\x00\x08\x04\x07\x07\x03\x02\x02\x04\x00\x07\x05\x03\x06\x00\x02\x01"
         "\x01\x02\x00\x01\x01\x03\x08\x07\x09\x08\x07\x01\x03\x09\x03\x03\x05"
         "\x07\x06\x05\x08\x07\x08\x09\x07\x06\x08\x08\x01\x04\x04\x01\x06\x06"
         "\x02\x02\x04\x09\x02\x08\x04\x07\x04\x03\x00\x06\x03\x09\x04\x07\x04"
         "\x01\x02\x04\x03\x07\x07\x07\x06\x07\x08\x09\x03\x04\x02\x04\x08\x06"
         "\x05\x04\x08\x05\x02\x07\x06\x03\x00\x02\x02\x01\x09\x06\x00\x01\x02"
         "\x04\x06\x00\x09\x04\x01\x01\x09\x04\x05\x03\x00\x08\x02\x09\x05\x02"
         "\x00\x08\x05\x00\x00\x05\x07\x06\x08\x08\x03\x08\x01\x05\x00\x06\x08"
         "\x02\x03\x04\x02\x04\x06\x02\x08\x08\x01\x04\x07\x03\x09\x01\x03\x01"
         "\x01\x00\x05\x04\x00\x08\x02\x07\x02\x03\x07\x01\x06\x03\x03\x05\x00"
         "\x05\x01\x00\x06\x08\x04\x05\x08\x06\x02\x09\x08\x02\x03\x09\x09\x04"
         "\x07\x02\x04\x05\x09\x03\x08\x04\x07\x09\x07\x01\x06\x03\x00\x04\x08"
         "\x03\x05\x03\x05\x06\x03\x02\x09\x06\x02\x04\x02\x02\x04\x01\x03\x07"
         "\x02\x01\x06"},
        {617,
         "\x03\x02\x03\x01\x07\x00\x00\x06\x00\x07\x01\x03\x01\x01\x00\x00\x07"
         "\x03\x00\x00\x07\x01\x04\x08\x07\x06\x06\x08\x08\x06\x06\x09\x09\x05"
         "\x01\x09\x06\x00\x04\x04\x04\x01\x00\x02\x06\x06\x09\x07\x01\x05\x04"
         "\x08\x04\x00\x03\x02\x01\x03\x00\x03\x04\x05\x04\x02\x07\x05\x02\x04"
         "\x06\x05\x05\x01\x03\x08\x08\x06\x07\x08\x09\x00\x08\x09\x03\x01\x09"
         "\x07\x02\x00\x01\x04\x01\x01\x05\x02\x02\x09\x01\x03\x04\x06\x03\x06"
         "\x08\x08\x07\x01\x07\x09\x06\x00\x09\x02\x01\x08\x09\x08\x00\x01\x09"
         "\x04\x09\x04\x01\x01\x09\x05\x05\x09\x01\x05\x00\x04\x09\x00\x09\x02"
         "\x01\x00\x09\x05\x00\x08\x08\x01\x05\x02\x03\x08\x06\x04\x04\x08\x02"
         "\x08\x03\x01\x02\x00\x06\x03\x00\x08\x07\x07\x03\x06\x07\x03\x00\x00"
         "\x09\x09\x06\x00\x09\x01\x07\x05\x00\x01\x09\x07\x07\x05\x00\x03\x08"
         "\x09\x06\x05\x02\x01\x00\x06\x07\x09\x06\x00\x05\x07\x06\x03\x08\x03"
         "\x08\x04\x00\x06\x07\x05\x06\x08\x02\x07\x06\x07\x09\x02\x02\x01\x08"
         "\x06\x04\x02\x06\x01\x09\x07\x05\x06\x01\x06\x01\x08\x03\x08\x00\x09"
         "\x04\x03\x03\x08\x04\x07\x06\x01\x07\x00\x04\x07\x00\x05\x08\x01\x06"
         "\x04\x05\x08\x05\x02\x00\x03\x06\x03\x00\x05\x00\x04\x02\x08\x08\x07"
         "\x05\x07\x05\x08\x09\x01\x05\x04\x01\x00\x06\x05\x08\x00\x08\x06\x00"
         "\x07\x05\x05\x02\x03\x09\x09\x01\x02\x03\x09\x03\x00\x03\x08\x05\x05"
         "\x02\x01\x09\x01\x04\x03\x03\x03\x03\x08\x09\x06\x06\x08\x03\x04\x02"
         "\x04\x02\x00\x06\x08\x04\x09\x07\x04\x07\x08\x06\x05\x06\x04\x05\x06"
         "\x09\x04\x09\x04\x08\x05\x06\x01\x07\x06\x00\x03\x05\x03\x02\x06\x03"
         "\x02\x02\x00\x05\x08\x00\x07\x07\x08\x00\x05\x06\x05\x09\x03\x03\x01"
         "\x00\x02\x06\x01\x09\x02\x07\x00\x08\x04\x06\x00\x03\x01\x04\x01\x05"
         "\x00\x02\x05\x08\x05\x09\x02\x08\x06\x04\x01\x07\x07\x01\x01\x06\x07"
         "\x02\x05\x09\x04\x03\x06\x00\x03\x07\x01\x08\x04\x06\x01\x08\x05\x07"
         "\x03\x05\x07\x05\x09\x08\x03\x05\x01\x01\x05\x02\x03\x00\x01\x06\x04"
         "\x05\x09\x00\x04\x04\x00\x03\x06\x09\x07\x06\x01\x03\x02\x03\x03\x02"
         "\x08\x07\x02\x03\x01\x02\x02\x07\x01\x02\x05\x06\x08\x04\x07\x01\x00"
         "\x08\x02\x00\x02\x00\x09\x07\x02\x05\x01\x05\x07\x01\x00\x01\x07\x02"
         "\x06\x09\x03\x01\x03\x02\x03\x04\x06\x09\x06\x07\x08\x05\x04\x02\x05"
         "\x08\x00\x06\x05\x06\x06\x09\x07\x09\x03\x05\x00\x04\x05\x09\x09\x07"
         "\x02\x06\x08\x03\x05\x02\x09\x09\x08\x06\x03\x08\x02\x01\x05\x05\x02"
         "\x05\x01\x06\x06\x03\x08\x09\x04\x03\x07\x03\x03\x05\x05\x04\x03\x06"
         "\x00\x02\x01\x03\x05\x04\x03\x03\x02\x02\x09\x06\x00\x04\x06\x04\x05"
         "\x03\x01\x08\x04\x07\x08\x06\x00\x04\x09\x05\x02\x01\x04\x08\x01\x09"
         "\x03\x05\x05\x05\x08\x05\x03\x06\x01\x01\x00\x05\x09\x05\x09\x06\x02"
         "\x03\x00\x06\x05\x06"},
    };
    if (x == 0) {
        if (big_alloc(r, 1) < 0) return -1;
        r->s[0] = 1;
        r->len = 1;
    }

    big tmp = {1, "\x01", 0};
    big *save = r, *fac = &tmp;
    for (int i = 0; x; i++) {
        if (x & 1) {
            if (big_mul(fac, &powtab[i], save) < 0) {
                big_free(fac);
                return -1;
            }
            big *swap_save = save;
            save = fac;
            fac = swap_save;
            big_free(save);
        }
        x >>= 1;
    }
    if (fac != r) {
        // replace it
        r->len = tmp.len;
        r->s = tmp.s;
        r->mem = tmp.mem;
    }
    return 0;
}

// calc a ^ x
use_retval static int big_pow_fast(unsigned a, unsigned x, BIGNIL big *r)
{
    if (x == 0) {
        if (big_alloc(r, 1) < 0) return -1;
        r->s[0] = 1;
        return 0;
    }

    big_new(base);
    big_push_u64(&base, a);

    big tmp = {1, "\x01", 0};
    big *save = r, *fac = &tmp;
    while (x) {
        if (x & 1) {
            if (big_mul(fac, &base, save) < 0) goto fail;
            big *swap_save = save;
            save = fac;
            fac = swap_save;
            big_free(save);
        }
        if (big_mul_self(&base) < 0) goto fail;
        x >>= 1;
    }
    if (fac != r) {
        // replace r
        r->len = tmp.len;
        r->s = tmp.s;
        r->mem = tmp.mem;
    }
    big_free(&base);
    return 0;
fail:
    big_free(&base);
    big_free(fac);
    return -1;
}

use_retval static int big_a_mul_pow2(big *a, int e, BIGNIL big *r)
{
    big_nil(pow);
    if (big_pow2(e, &pow) < 0) return -1;
    if (big_mul(a, &pow, r) < 0) {
        big_free(&pow);
        return -1;
    }
    big_free(&pow);
    return 0;
}

static inline void _big_get_mid(const big *lo, const big *hi, big *mid)
{
    big_copy(lo, mid);
    big_pad(mid, hi->len);
    big_add(mid, hi);

    int carry = 1;
    for (int i = mid->len - 1; i >= 0 && carry; i--) {
        if (mid->s[i] == 9)
            mid->s[i] = 0;
        else
            mid->s[i]++, carry = 0;
    }
    if (carry) {
        *--mid->s = 1;
        mid->len++;
    }

    carry = 0;
    for (int i = 0; i < mid->len; i++) {
        int cur = carry * 10 + mid->s[i];
        mid->s[i] = cur / 2;
        carry = cur & 1;
    }
    big_trim_zero(mid);
}

use_retval static int big_div_rem(big *a, big *b, big *quo, big *rem)
{
    big_nil(tmp);
    big_new(lo);
    big_new(hi);
    big_new(mid);
    char *const mid_s_orig = mid.s;

    if (big_alloc(rem, a->len) < 0) return -1;
    big_copy(a, rem);

    quo->len = 0;
    if (!rem->len || !big_ge(rem, b)) return 0;

    // hi = 10^(rem->len - b->len + 1)
    int qlen = rem->len - b->len + 1;
    hi.len = qlen + 1;
    hi.s[0] = 1;
    for (int i = 1; i < hi.len; i++) hi.s[i] = 0;

    // lo = 1
    lo.len = 1;
    lo.s[0] = 1;

    while (!big_ge(&lo, &hi)) {
        mid.s = mid_s_orig;
        _big_get_mid(&lo, &hi, &mid);

        // test = mid * b
        big_free(&tmp);
        if (big_mul(&mid, b, &tmp) < 0) goto fail;

        if (big_ge(rem, &tmp)) {
            // lo = mid
            big_copy(&mid, &lo);
        } else {
            // hi = mid - 1
            big_copy(&mid, &hi);
            if (hi.len) {
                int i = hi.len - 1;
                while (i >= 0 && hi.s[i] == 0) {
                    hi.s[i] = 9;
                    i--;
                }
                if (i >= 0) hi.s[i]--;
                big_trim_zero(&hi);
            }
        }
    }

    // quo = lo
    if (big_alloc(quo, lo.len) < 0) goto fail;
    big_copy(&lo, quo);

    // rem = a - quo * b
    big_free(&tmp);
    if (big_mul(quo, b, &tmp) < 0) goto fail;
    big_sub(rem, &tmp);
    big_free(&tmp);
    return 0;

fail:
    big_free(&tmp);
    big_free(rem);
    big_free(quo);
    return -1;
}

// 0, 1, ... [from, ...
static int big_dec_round(big *r, int *dot, int from, bool dry)
{
    if (from >= r->len) return 0;
    assert(!big_is_zero(r));
    assert(from >= 0);
    int carry = 0;
    if (r->s[from] > 5)
        carry = 1;
    else if (from && r->s[from] == 5 && (r->s[from - 1] & 1))
        carry = 1;
    for (int j = 0; j < from; j++) {
        int curr = r->s[from - j - 1] + carry;
        if (curr >= 10) {
            curr -= 10;
            carry = 1;
        } else
            carry = 0;
        if (!dry) r->s[from - j - 1] = curr;
    }
    if (!dry) {
        if (carry) {
            *--r->s = 1;
            ++r->len;
            ++(*dot);
        }
        for (int j = from; j < r->len; j++)
            r->s[j] = 0;
        while (!r->s[r->len - 1] && r->len > *dot)
            r->len--;
    }
    return carry;
}

static int big_first_sig(const big *a)
{
    assert(!big_is_zero(a));
    int first_sig = 0;
    while (!a->s[first_sig])
        first_sig++;
    return first_sig;
}

static int big_sci_get_X(big *a, int dot, int prec)
{
    int first_sig = big_first_sig(a);
    int e10 = dot - (first_sig + 1);
    int erupt = big_dec_round(a, &dot, first_sig + prec, true);
    return e10 + erupt;
}

// assert !big_is_zero(a)
static int big_sci_converter(big *a, int *dot, int *prec)
{
    int first_sig = big_first_sig(a);
    int e10 = *dot - (first_sig + 1);
    a->s += first_sig;
    a->len -= first_sig;
    *dot = 1;
    // 9.9 x 10^-2 -> 10 x 10^-2 -> 1 x 10^-1
    if (big_dec_round(a, dot, *prec, false)) {
        *dot = 1;
        e10++;
    }
    *prec -= a->len;
    assert(a->len > 0);
    return e10;
}

static int big_fixed_converter(big *a, int *dot, int *prec)
{
    big_dec_round(a, dot, *dot + *prec, false);
    *prec -= a->len - *dot;
    return 0;
}

use_retval static int big_a_div_pow2(big *a, int e, BIGNIL big *r, int *dot,
                                     int prec, bool need_sigprec)
{
    assert(need_sigprec ? prec > 0 : 1);
    int ret = 0;
    bool need_rnd = false;
    big_nil(pow);
    if (big_pow2(e, &pow) < 0) goto err;
    big_nil(tmp);
    big_new(mid_big);
    big_nil(spow);

    // integer part via binary search
    uint64_t inte = 0;
    if (big_ge(a, &pow)) {
        uint64_t lo = 1, hi = 1ULL << 53;
        while (lo < hi) {
            uint64_t mid = lo + (hi - lo + 1) / 2;
            mid_big.len = 0;
            big_push_u64(&mid_big, mid);
            big_free(&tmp);
            if (big_mul(&mid_big, &pow, &tmp) < 0) goto err;
            if (big_ge(a, &tmp))
                lo = mid;
            else
                hi = mid - 1;
        }
        inte = lo;
        mid_big.len = 0;
        big_push_u64(&mid_big, inte);
        big_free(&tmp);
        if (big_mul(&mid_big, &pow, &tmp) < 0) goto err;
        big_sub(a, &tmp);
    }
    *dot = big_push_u64(r, inte);
    if (big_is_zero(a)) goto cleanup;
    if (big_extend(a, e) < 0) goto err;

    // 计算小数点后 prec + 1 位, 或者计算有效数字 prec + 1 位
    // (need_sigprec 时) 二者取最大
    for (int k = 0, nsig = *dot;; k++) {
        if ((need_sigprec ? nsig >= prec + 1 : true) && k >= prec + 1) break;
        a->s[a->len++] = 0;
        if (!big_ge(a, &pow)) {
            r->s[r->len++] = 0;
            if (nsig > 0) nsig++;
            continue;
        }
        int lo = 1, hi = 9;
        while (lo < hi) {
            int mid = (lo + hi + 1) >> 1;
            big_free(&spow);
            if (big_mul_small(&pow, mid, &spow) < 0) goto err;
            if (big_ge(a, &spow))
                lo = mid;
            else
                hi = mid - 1;
        }
        if (lo != 0 || nsig > 0) nsig++;
        r->s[r->len++] = lo;
        big_free(&spow);
        if (big_mul_small(&pow, lo, &spow) < 0) goto err;
        big_sub(a, &spow);
        if (a->len == 0) break;
    }

cleanup:
    big_free(&tmp);
    big_free(&spow);
    big_free(&pow);
    return ret;
err:
    ret = -1;
    goto cleanup;
}

#include <stdio.h>

static void big_debug_dump(big *a)
{
    char *buf;
    if (big_tostr(&buf, a, -1) < 0) {
        fputs("error converting", stderr);
        return;
    }
    fputs(buf, stderr);
    fputs("\n", stderr);
    free(buf);
}
