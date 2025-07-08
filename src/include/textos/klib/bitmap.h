#pragma once

// do not change it!
#define mpunit u8
#define mpfull (mpunit)-1
#define mpemsz sizeof(mpunit)
#define mpembs (mpemsz * 8)

typedef struct bitmap
{
    mpunit *map;
    size_t siz;
} bitmap_t;

bitmap_t *bitmap_init(bitmap_t *bmp, void *buf, size_t siz);

size_t bitmap_find(bitmap_t *bmp);

bool bitmap_test(bitmap_t *bmp, size_t i);

void bitmap_set(bitmap_t *bmp, size_t i);

void bitmap_reset(bitmap_t *bmp, size_t i);

void bitmap_clear(bitmap_t *bmp);

#define bitmap_exist(bmp, i) (bmp->siz > i)
