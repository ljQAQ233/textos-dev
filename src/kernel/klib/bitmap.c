#include <textos/mm/heap.h>
#include <textos/klib/bitmap.h>
#include <textos/klib/string.h>

bitmap_t *bitmap_init(bitmap_t *bmp, void *buf, size_t bits)
{
    void *orig_buf = buf;
    size_t bytes = DIV_ROUND_UP(bits, mpembs);
    if (!buf && !(buf = malloc(bytes))) {
        return NULL;
    }
    if (!bmp) {
        if (!(bmp = malloc(sizeof(bitmap_t)))) {
            if (!orig_buf) free(buf);
            return NULL;
        }
        memset(buf, 0, bytes);
    }
    bmp->map = buf;
    bmp->bits = bits;
    return bmp;
}

// x != (-1) => ~x != 0
// ffs((mpunit)0) = 1   -> not exists
// ffs((mpunit)-1) = 1  -> possible
static int poplow(mpunit x)
{
    x = ~x;
    return __builtin_ffs(x) - 1;
}

#define SET(x, i) (x |=  (1 << i))
#define CLR(x, i) (x &= ~(1 << i))
#define TEST(x, i) (x &  (1 << i))

size_t bitmap_find_range(bitmap_t *bmp, size_t from, size_t to);

size_t bitmap_find(bitmap_t *bmp)
{
    return bitmap_find_range(bmp, 0, bmp->bits);
}

size_t bitmap_find_from(bitmap_t *bmp, size_t from)
{
    return bitmap_find_range(bmp, from, bmp->bits);
}

/*
 * find an available bit in the range [from, to)
 */
size_t bitmap_find_range(bitmap_t *bmp, size_t from, size_t to)
{
    ASSERTK(to <= bmp->bits);
    ASSERTK(from <= to);

    size_t bytes = DIV_ROUND_UP(bmp->bits, mpembs);
    size_t idx = from / mpembs;
    mpunit *p = bmp->map + idx;

    mpunit mask = (mpunit)((1 << (from % mpembs)) - 1);

    for (; idx < bytes; idx++, p++) {
        mpunit unit = *p | mask;
        if (unit == mpfull) {
            mask = 0;
            continue;
        }

        int low = poplow(unit);
        if (idx == bytes - 1 && (bmp->bits % mpembs) != 0 &&
            low >= (bmp->bits % mpembs)) {
            return (size_t)-1;
        }

        SET(*p, low);
        return low + mpembs * idx;
    }

    return (size_t)-1;
}

bool bitmap_test(bitmap_t *bmp, size_t i)
{
    ASSERTK(bitmap_exist(bmp, i));
    size_t idx = i / mpembs;
    size_t low = i % mpembs;

    return TEST(bmp->map[idx], low) ? true : false;
}

void bitmap_set(bitmap_t *bmp, size_t i)
{
    ASSERTK(bitmap_exist(bmp, i));
    size_t idx = i / mpembs;
    size_t low = i % mpembs;
    SET(bmp->map[idx], low);
}

void bitmap_reset(bitmap_t *bmp, size_t i)
{
    ASSERTK(bitmap_exist(bmp, i));
    size_t idx = i / mpembs;
    size_t low = i % mpembs;
    CLR(bmp->map[idx], low);
}

void bitmap_clear(bitmap_t *bmp)
{
    memset(bmp->map, 0, DIV_ROUND_UP(bmp->bits, mpembs));
}
