/*
 * packet buffer
 */

#include <textos/klib/list.h>

#include <textos/dev/mbuf.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>

static list_t l_free = LIST_INIT(l_free);

mbuf_t *mbuf_alloc(int headroom)
{
    mbuf_t *m;

    if (list_empty(&l_free))
    {
        addr_t pp, vp;
        vmm_allocpv(1, &vp, &pp);
        m = (mbuf_t *)vp;
        m->phy = pp;
        m->len = 0;
        m->next = NULL;
        m->head = m->buf;
    }
    else
    {
        list_t *f = l_free.next;
        list_remove(f);
        m = CR(f, mbuf_t, list);
    }
    m->phy += headroom;
    m->head += headroom;

    return m;
}

void mbuf_free(mbuf_t *m)
{
    mbuf_t *i = m;
    mbuf_t *nxt = m;
    while (i)
    {
        nxt = i->next;
        int off = i->head - i->buf;
        i->head -= off;
        i->phy -= off;
        i->len = 0;
        list_insert(&l_free, &i->list);
        i = nxt;
    }
}

u8 *mbuf_push(mbuf_t *m, size_t len)
{
    m->head -= len;
    m->phy -= len;
    m->len += len;
    return m->head;
}

u8 *mbuf_pull(mbuf_t *m, size_t len)
{
    m->head += len;
    m->phy += len;
    m->len -= len;
    return m->head;
}

u8 *mbuf_put(mbuf_t *m, size_t len)
{
    u8 *p = m->head + m->len;
    m->len += len;
    return p;
}

