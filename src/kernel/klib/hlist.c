#include <textos/klib/hlist.h>

void hlist_init(hlist_head_t *h)
{
    h->first = NULL;
}

void hlist_add(hlist_head_t *h, hlist_node_t *n)
{
    hlist_node_t *first = h->first;
    n->next = first;
    if (first)
        first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

void hlist_del(hlist_node_t *n)
{
    if (n->next)
        n->next->pprev = n->pprev;
    *(n->pprev) = n->next;
    n->next = NULL;
    n->pprev = NULL;
}

