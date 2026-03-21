#pragma once

#include <stdint.h>
#include <malloc.h>
#include "hlist.h"

typedef uint64_t htkey_t;

typedef struct htable
{
    size_t size;
    hlist_head_t *tbl;
} htable_t;

static htable_t *htable_init(htable_t *ht, size_t size)
{
    if (!ht)
        ht = malloc(sizeof(htable_t));
    ht->size = size;
    ht->tbl = malloc(sizeof(hlist_head_t) * ht->size);
    for (int i = 0 ; i < ht->size ; i++)
        ht->tbl[i] = HLIST_INIT();
    return ht;
}

static void htable_add(htable_t *ht, hlist_node_t *node, htkey_t key)
{
    hlist_add(&ht->tbl[key % ht->size], node);
}

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
