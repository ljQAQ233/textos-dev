
// disk buffer
// rbtree : todo
// simple LRU (Least Recently Used)

#include <irq.h>
#include <textos/mm.h>
#include <textos/dev/buffer.h>

static list_t all;
static list_t lru;

static lock_t lkall;

static buffer_t *find_all(dev_t *dev, int idx)
{
    buffer_t *c;

    list_t *p;
    LIST_FOREACH(p, &all)
    {
        c = CR(p, buffer_t, all);
        if (c->dev == dev && c->idx == idx)
            return c;
    }
    return NULL;
}

// read a block and use exclusively
buffer_t *bread(dev_t *dev, size_t idx)
{
    buffer_t *b;
    lock_acquire(&lkall);

    b = find_all(dev, idx);
    if (b == NULL) {
        b = malloc(sizeof(buffer_t));
        b->dev = dev;
        b->idx = idx;
        b->siz = 512;
        b->blk = malloc(b->siz);
        b->dirty = false;
        list_insert(&all, &b->all);
        lock_init(&b->lock);

        dev->bread(dev, idx, b->blk, 1);
    }

    lock_acquire(&b->lock);
    lock_release(&lkall);
    return b;
}

void bdirty(buffer_t *b, bool dirty)
{
    b->dirty = dirty;
}

// synchronize changes to disk
void bwrite(buffer_t *b)
{
    if (!b->dirty)
        return;

    dev_t *dev = b->dev;
    dev->bwrite(dev, b->idx, b->blk, 1);
}

// allow others to access the block
void brelse(buffer_t *b)
{
    bwrite(b);
    lock_release(&b->lock);
}

void buffer_init()
{
    list_init(&all);
    lock_init(&lkall);
    DEBUGK(K_INIT, "buffer initialized!\n");
}
