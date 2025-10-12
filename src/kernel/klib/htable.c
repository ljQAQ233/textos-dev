#include <textos/mm/heap.h>
#include <textos/klib/htable.h>

htable_t *htable_init(htable_t *ht, size_t size)
{
    if (!ht)
        ht = malloc(sizeof(htable_t));
    ht->size = size;
    ht->tbl = malloc(sizeof(hlist_head_t) * ht->size);
    for (int i = 0 ; i < ht->size ; i++)
        ht->tbl[i] = HLIST_INIT();
    return ht;
}

void htable_add(htable_t *ht, hlist_node_t *node, htkey_t key)
{
    hlist_add(&ht->tbl[key % ht->size], node);
}
