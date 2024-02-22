#pragma once

typedef struct
{
    void    *buf;      // main body
    size_t  elem_siz;  // element size
    size_t  head;      // head index
    size_t  tail;      // tail index
    size_t  max;       // max index   
    size_t  siz;       // the ring buffer siz
} ring_t;

/* MARK:Use the feature of overflow may gain more improvements */

ring_t *ring_init (ring_t *r, void *buf, size_t siz, size_t elem_siz);

void *ring_get (ring_t *r, size_t idx);

void *ring_pop (ring_t *r);

void ring_push (ring_t *r, void *elem);

bool ring_empty (ring_t *r);

void ring_clear (ring_t *r);

