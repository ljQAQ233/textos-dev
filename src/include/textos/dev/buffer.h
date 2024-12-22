#pragma once

#include <textos/dev.h>
#include <textos/lock.h>
#include <textos/klib/list.h>

typedef struct
{
    bool dirty;
    size_t idx;
    size_t siz;
    void *blk;
    dev_t *dev;
    lock_t lock;
    list_t all;
} buffer_t;

buffer_t *bread(dev_t *dev, size_t idx);

void bdirty(buffer_t *b, bool dirty);

void bwrite(buffer_t *b);

void brelse(buffer_t *b);

