#include <textos/klib/bitmap.h>
#include <textos/mm.h>
#include <textos/assert.h>

#include <string.h>

bitmap_t *bitmap_init(bitmap_t *bmp, void *buf, size_t siz)
{
    ASSERTK(siz != 0 && siz != -1);

    if (!buf && !(buf = malloc(siz)))
    {
        memset(buf, 0, siz / mpembs);
        return NULL;
    }
    
    if (!bmp && !(bmp = malloc(sizeof(bitmap_t))))
        return NULL;

    bmp->map = buf;
    bmp->siz = siz;
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

size_t bitmap_find(bitmap_t *bmp)
{
    size_t max = bmp->siz / mpembs;
    mpunit *p  = bmp->map;

    size_t idx;
    for (idx = 0 ; idx < max ; idx++, p++)
    {
        if (*p == mpfull)
            continue;
        break;
    }

    if (idx == max)
        return (size_t)-1;
    
    int low = poplow(*p);
    SET(*p, low);

    return low + mpembs * idx;
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
    memset(bmp->map, 0, bmp->siz / mpembs);
}
