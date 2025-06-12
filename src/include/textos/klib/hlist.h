#pragma once

typedef struct hlist_node
{
    struct hlist_node *next;
    struct hlist_node **pprev;
} hlist_node_t;

typedef struct hlist_head
{
    struct hlist_node *first;
} hlist_head_t;

void hlist_init(hlist_head_t *h);

void hlist_add(hlist_head_t *h, hlist_node_t *n);

void hlist_del(hlist_node_t *n);

#define HLIST_INIT() ((hlist_head_t){ .first = NULL })

#define HLIST_FOREACH(ptr, hlist) \
    for (ptr = (hlist)->first ; ptr ; ptr = ptr->next)
