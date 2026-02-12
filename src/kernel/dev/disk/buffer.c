/**
 * @brief blockdev buffer
 * 
 * TODO: fix lock
 * TODO: simple LRU (Least Recently Used)
 */

#include <irq.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>
#include <textos/dev/buffer.h>
#include <textos/klib/htable.h>

htable_define(all, 32);

static list_t fre;
static list_t lru;

static lock_t lkall;

static inline htkey_t hash(devst_t *dev, int idx)
{
    return dev->major ^ idx;
} 

static buffer_t *find_all(devst_t *dev, int idx)
{
    buffer_t *b;
    hlist_node_t *p;
    htkey_t key = hash(dev, idx);
    HTABLE_FOREACH(p, &all, key)
    {
        b = CR(p, buffer_t, node);
        if (b->dev == dev && b->idx == idx)
            return b;
    }
    return NULL;
}

/**
 * @brief add some available pages into free list
 * 
 * @param bsiz must be a power of 2
 */
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
        list_insert(&fre, &n->list);
    }
}

static void mark_fre(buffer_t *b)
{
    list_insert(&fre, &b->list);
}

#include <textos/panic.h>

static buffer_t *find_fre(size_t bsiz)
{
    if (list_empty(&fre))
        fill_fre(bsiz);

    buffer_t *r = NULL;
    list_t *ptr;
    LIST_FOREACH(ptr, &fre)
    {
        buffer_t *b = CR(ptr, buffer_t, list);
        if (b->siz == bsiz)
        {
            r = b;
            list_remove(ptr);
            break;
        }
    }

    if (r == NULL)
    {
        fill_fre(bsiz);
        r = find_fre(bsiz);
    }
    return r;
}

// read a block and use exclusively
buffer_t *bread(devst_t *dev, blksize_t siz, blkno_t idx)
{
    buffer_t *b;
    lock_acquire(&lkall);

    b = find_all(dev, idx);
    if (b == NULL)
    {
        b = find_fre(siz);
        b->dev = dev;
        b->idx = idx;
        b->dirty = false;
        lock_init(&b->lock);

        /* siz >= bsiz */
        int bsiz;
        dev->ioctl(dev, BLKSSZGET, &bsiz);
        int scale = siz / bsiz;
        dev->bread(dev, idx * scale, b, siz / bsiz);

        htkey_t key = hash(dev, idx);
        htable_add(&all, &b->node, key);
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
    int bsiz;
    dev->ioctl(dev, BLKSSZGET, &bsiz);
    int scale = b->siz / bsiz;
    dev->bwrite(dev, b->idx * scale, b, b->siz / bsiz);
}

// allow others to access the block
void brelse(buffer_t *b)
{
    bwrite(b);
    lock_release(&b->lock);
}

buffer_t *bdma_alloc(blksize_t siz)
{
    buffer_t *b = find_fre(siz);
    b->dev = 0;
    b->idx = 0;
    b->dirty = false;
    lock_init(&b->lock);
    lock_acquire(&b->lock);
    return b;
}

void bdma_free(buffer_t *b)
{
    lock_release(&b->lock);
    mark_fre(b);
}

void buffer_init()
{
    list_init(&fre);
    lock_init(&lkall);
    DEBUGK(K_INFO, "buffer initialized!\n");
}
