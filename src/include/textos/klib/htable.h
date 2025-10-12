#pragma once

#include <textos/klib/hlist.h>

typedef u64 htkey_t;

typedef struct htable
{
    size_t size;
    hlist_head_t *tbl;
} htable_t;

htable_t *htable_init(htable_t *ht, size_t size);

void htable_add(htable_t *ht, hlist_node_t *node, htkey_t key);

#define __htable_tbl(x) x##_##tbl

/**
 * @brief define a local hash table
 */
#define htable_define(name, tblsz) \
    static hlist_head_t __htable_tbl(name)[tblsz] = { 0 }; \
    static htable_t name = {                               \
        .size = (tblsz),                                   \
        .tbl = __htable_tbl(name) };                       \

#define HTABLE_FOREACH(ptr, ht, key) \
    HLIST_FOREACH(ptr, &(ht)->tbl[(key) % (ht)->size])

#define HTABLE_FORANY(ptr, ht) \
    for (int __i = 0 ; __i < (ht)->size ; __i++) \
        HLIST_FOREACH(ptr, &(ht)->tbl[__i])
