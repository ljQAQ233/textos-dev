#include <textos/klib/ring.h>
#include <textos/mm.h>
#include <textos/assert.h>

#include <string.h>

/* If Buffer is not provided, allocate memory automatically */
ring_t *ring_init (ring_t *r, void *buf, size_t siz, size_t elem_siz)
{
    ASSERTK (siz != 0);

    // if (Siz / Elem == 0)
    //     return NULL;

    if (!buf && !(buf = malloc(siz)))
        return NULL;
    
    if (!r && !(r = malloc(sizeof(ring_t))))
        return NULL;
    
    r->buf = buf;
    r->siz = siz;
    r->elem_siz = elem_siz;
    r->max  = siz / elem_siz;

    ring_clear(r);

    return r;
}

/* Fix index (adjust it into correct one) */
static size_t ring_fixi (ring_t *r, size_t *idx)
{
    *idx %= r->max;
    return *idx;
}

/* Get the target by index */
void *ring_get (ring_t *r, size_t idx)
{
    ring_fixi (r, &idx);
    return r->buf + r->elem_siz * idx;
}

void *ring_head (ring_t *r)
{
    return ring_get (r, ring_fixi (r, &r->head));
}

void *ring_tail (ring_t *r)
{
    return ring_get (r, ring_fixi (r, &r->tail));
}

void *ring_pop (ring_t *r)
{
    /* Fetch it and update info */
    void *elem = ring_head (r);

    ring_fixi (r, &r->head);

    r->head++;

    return elem;
}

void ring_push (ring_t *r, void *elem)
{
    ring_fixi (r, &r->tail);

    memcpy (ring_get (r, r->tail), elem, r->elem_siz);
    r->tail++;
}

bool ring_empty (ring_t *r)
{
    return ring_fixi (r, &r->head) == ring_fixi (r, &r->tail);
}

void ring_clear (ring_t *r)
{
    memset (r->buf, 0, r->max);

    r->head = 0;
    r->tail = 0;
}

