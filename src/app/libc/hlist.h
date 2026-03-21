/*
 * hash list
 */
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

static void hlist_init(hlist_head_t *h)
{
    h->first = 0;
}

static void hlist_add(hlist_head_t *h, hlist_node_t *n)
{
    hlist_node_t *first = h->first;
    n->next = first;
    if (first) first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

static void hlist_del(hlist_node_t *n)
{
    if (n->next) n->next->pprev = n->pprev;
    *(n->pprev) = n->next;
    n->next = 0;
    n->pprev = 0;
}

#define HLIST_INIT() ((hlist_head_t){.first = 0})

#define HLIST_FOREACH(ptr, hlist) \
    for (ptr = (hlist)->first; ptr; ptr = ptr->next)
