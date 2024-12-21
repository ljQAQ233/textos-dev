
// simple LRU (Least Recently Used)

#include <textos/mm.h>
#include <textos/lock.h>
#include <textos/klib/list.h>

typedef struct
{
    addr_t idx;
    list_t all;
    void *bs; // buffer struct
    void *data;
    lock_t lock;
} buffer_t;

static list_t bfree;
static lock_t bfree_lock;

buffer_t *alloc()
{
    lock_acquire(&bfree_lock);

    buffer_t *buf;
    if (list_empty(&bfree)) {
        buf = malloc(sizeof(buffer_t));
    } else {
        list_t *back = bfree.back;
        buf = CR(back, buffer_t, all);
        list_remove(back);
    }

    lock_release(&bfree_lock);
    return buf;
}

void *free(buffer_t *buf)
{

}

void buffer_init()
{
    list_init(&free);
}
