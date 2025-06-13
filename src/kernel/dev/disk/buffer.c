
// disk buffer
// rbtree : todo
// simple LRU (Least Recently Used)

#include <irq.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>
#include <textos/dev/buffer.h>

static list_t all;
static list_t fre;
static list_t lru;

static lock_t lkall;

static buffer_t *find_all(devst_t *dev, int idx)
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

static void fill_fre(size_t bsiz)
{
    void *vp;
    addr_t pp;
    int nr = 1, x = PAGE_SIZ / bsiz;
    if (x == 0)
    {
        nr = bsiz / PAGE_SIZ;
        x = 1;
    }

    vmm_allocpv(nr, (addr_t *)&vp, &pp);
    for (int i = 0 ; i < x ; i++)
    {
        buffer_t *n = malloc(sizeof(buffer_t));
        n->siz = bsiz;
        n->blk = vp + i * n->siz;
        n->phy = pp + i * n->siz;
        list_insert(&fre, &n->all);
    }
}

static void mark_fre(buffer_t *b)
{
    list_insert(&fre, &b->all);
}

#include <textos/panic.h>

static buffer_t *find_fre(size_t bsiz)
{
    if (list_empty(&fre))
        fill_fre(bsiz);

    list_t *ptr;
    LIST_FOREACH(ptr, &fre)
    {
        buffer_t *b = CR(ptr, buffer_t, all);
        if (b->siz == bsiz)
        {
            list_remove(ptr);
            return b;
        }
    }

    __builtin_unreachable();
}

// read a block and use exclusively
buffer_t *bread(devst_t *dev, size_t idx)
{
    buffer_t *b;
    lock_acquire(&lkall);

    size_t bsiz = 512;
    b = find_all(dev, idx);
    if (b == NULL)
    {
        b = find_fre(bsiz);
        b->dev = dev;
        b->idx = idx;
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

    devst_t *dev = b->dev;
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
    list_init(&fre);
    lock_init(&lkall);
    DEBUGK(K_INIT, "buffer initialized!\n");
}
