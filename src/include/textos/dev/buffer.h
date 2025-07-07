#pragma once

#include <textos/fs.h>
#include <textos/dev.h>
#include <textos/lock.h>
#include <textos/klib/list.h>
#include <textos/klib/hlist.h>

typedef struct
{
    bool dirty;
    blkno_t idx;
    blksize_t siz;
    void *blk;
    addr_t phy;
    devst_t *dev;
    lock_t lock;
    list_t list;
    hlist_node_t node;
} buffer_t;

buffer_t *bread(devst_t *dev, blksize_t siz, blkno_t idx);

void bdirty(buffer_t *b, bool dirty);

void bwrite(buffer_t *b);

void brelse(buffer_t *b);

static inline buffer_t *sb_bread(superblk_t *sb, blkno_t idx)
{
    return bread(sb->dev, sb->blksz, idx);
}
