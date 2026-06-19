#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

static bool big_is_zero(big *a)
{
    return !a->len;
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

static void big_alloc(BIGNIL big *a, unsigned len)
{
    assert(a->mem == NULL);
    a->mem = malloc(len + BIGHOLE);
    assert(a->mem != NULL);
    a->len = len;
    a->s = a->mem + BIGHOLE;
}

static void big_extend(big *a, unsigned addition)
{
    void *mem = malloc(a->len + addition + BIGHOLE);
    void *s = mem + BIGHOLE;
    memcpy(s, a->s, a->len);
    a->mem = mem;
    a->s = s;
}

static void big_trim_zero(big *a)
{
    int z = 0;
    while (!a->s[z] && z < a->len)
        z++;
    memmove(a->s, a->s + z, a->len - z);
    a->len -= z;
}

static void big_mul_small(const big *a, int d, BIGNIL big *r)
{
    big_alloc(r, a->len + 1);
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
}

static int big_tostr(char **buf, big *a, int dot)
{
    *buf = malloc(a->len + 4);
    char *p = *buf;
    for (int i = 0; i < a->len; i++) {
        if (dot == i) {
            if (i == 0) *p++ = '0';
            *p++ = '.';
        }
        *p++ = ('0' + a->s[i]);
    }
    *p++ = '\0';
    return p - *buf - 1;
}

static int big_tohex(char **buf, big *a, bool lower, int *prec)
{
    static const char upstr[] = "0123456789ABCDEF";
    static const char lwstr[] = "0123456789abcdef";
    const char *letters = !lower ? upstr : lwstr;
    int len = a->len;
    if (len == 0) return 0;

    // FIXME: all malloc need error handling
    char *hex = malloc(len + 1 + 1);
    int hexsz = 0;

    char *num = malloc(len);
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

    // 约取
    if (*prec < 0) {
        *prec = 0;
    } else if (hexsz > *prec) {
        if (hex[*prec] > 8)
            hex[*prec - 1]++;
        else if (hex[*prec] == 8 && (hex[*prec - 1] & 1))
            hex[*prec - 1]++;
        hexsz = *prec;
        *prec = 0;
        int carry = 0;
        for (int i = *prec; i >= 0; i--) {
            hex[i] += carry;
            if (hex[i] >= 16) {
                hex[i] -= 16;
                carry = 1;
            } else {
                carry = 0;
            }
        }
        if (carry) {
            memmove(hex + 1, hex, hexsz);
            hex[0] = 1;
        }
    } else {
        *prec -= hexsz;
    }

    for (int i = 0; i < hexsz; i++)
        hex[i] = letters[hex[i]];
    *buf = hex;
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

// calc a x b -> r
// assert (a) x (b) != 0
static void big_mul(big *a, const big *b, BIGNIL big *r)
{
    big_alloc(r, a->len + b->len);
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
}

// calc 2 ^ x -> r (tested, resonating with python)
static void big_pow2(unsigned x, BIGNIL big *r)
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
        big_alloc(r, 1);
        r->s[0] = 1;
        r->len = 1;
    }

    big tmp = {1, "\x01", 0};
    big *save = r, *fac = &tmp;
    for (int i = 0; x; i++) {
        if (x & 1) {
            big_mul(fac, &powtab[i], save);
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
}

static void big_a_mul_pow2(big *a, int e, BIGNIL big *r)
{
    big_nil(pow);
    big_pow2(e, &pow);
    big_mul(a, &pow, r);
    big_free(&pow);
}

static void big_a_div_pow2(big *a, int e, BIGNIL big *r, int *dot, int prec)
{
    big_nil(pow);
    big_pow2(e, &pow);
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
            big_mul(&mid_big, &pow, &tmp);
            if (big_ge(a, &tmp))
                lo = mid;
            else
                hi = mid - 1;
        }
        inte = lo;
        mid_big.len = 0;
        big_push_u64(&mid_big, inte);
        big_free(&tmp);
        big_mul(&mid_big, &pow, &tmp);
        big_sub(a, &tmp);
    }
    *dot = big_push_u64(r, inte);
    if (a->len == 0) goto cleanup;
    big_extend(a, e);

    // calc (prec + 1) numbers after the dot
    for (int k = 0; k < prec + 1; k++) {
        a->s[a->len++] = 0;
        if (!big_ge(a, &pow)) {
            r->s[r->len++] = 0;
            continue;
        }
        int lo = 1, hi = 9;
        while (lo < hi) {
            int mid = (lo + hi + 1) >> 1;
            big_free(&spow);
            big_mul_small(&pow, mid, &spow);
            if (big_ge(a, &spow))
                lo = mid;
            else
                hi = mid - 1;
        }
        r->s[r->len++] = lo;
        big_free(&spow);
        big_mul_small(&pow, lo, &spow);
        big_sub(a, &spow);
        if (a->len == 0) break;
    }

    if (r->len - *dot == prec + 1) {
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

cleanup:
    big_free(&tmp);
    big_free(&spow);
    big_free(&pow);
}
