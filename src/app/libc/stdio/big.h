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

#define BIGHOLE  8
#define BIGNIL

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

static int big_tostr(char **buf, big *a, int dot)
{
    *buf = malloc(a->len + 4);
    char *p = *buf;
    for (int i = 0; i < a->len; i++) {
        if (dot == i) *p++ = ('.');
        *p++ = ('0' + a->s[i]);
    }
    *p++ = '\0';
    return p - *buf - 1;
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

static int big_push_u64(BIGNIL big *a, uint64_t num)
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

// TODO: handle nil
// assume pushing doesn't cause overflow
static int big_push_str(BIGNIL big *a, char *str)
{
    char *s = str;
    for (; *s; s++)
        a->s[a->len++] = *s - '0';
    big_trim_zero(a);
    return s - str;
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
}

static void big_a_div_pow2(big *a, int e, BIGNIL big *r, int *dot, int prec)
{
    big_nil(pow);
    big_pow2(e, &pow);
    
    // integer part
    // TODO: optimize
    int inte = 0;
    while (big_ge(a, &pow)) {
        big_sub(a, &pow);
        inte++;
    }
    *dot = big_push_u64(r, inte);
    if (a->len == 0) return;
    big_extend(a, e);

    // calc (prec + 1) numbers after the dot
    for (int k = 0; k < prec + 1; k++) {
        a->s[a->len++] = 0;
        if (!big_ge(a, &pow)) {
            r->s[r->len++] = 0;
            continue;
        }
        int current = 0;
        while (big_ge(a, &pow)) {
            big_sub(a, &pow);
            current++;
        }
        r->s[r->len++] = current;
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
}
