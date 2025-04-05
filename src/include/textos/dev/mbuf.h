#pragma once

struct mbuf;
typedef struct mbuf mbuf_t;

#define MBUF_SIZE    2048
#define MBUF_DEFROOM 128

#include <textos/klib/list.h>

struct mbuf
{
    u8 buf[MBUF_SIZE];
    u8 *head;
    mbuf_t *next;
    addr_t phy;
    size_t len;
    size_t dlen;
    size_t id;
    size_t flgs;
    list_t list;
    list_t wque; // waiters queue
};

mbuf_t *mbuf_alloc(int headroom);

void mbuf_free(mbuf_t *m);

u8 *mbuf_pull(mbuf_t *m, size_t len);

u8 *mbuf_push(mbuf_t *m, size_t len);

u8 *mbuf_put(mbuf_t *m, size_t len);

#define mbuf_pullhdr(m, type) ((type *)mbuf_pull(m, sizeof(type)))
#define mbuf_pushhdr(m, type) ((type *)mbuf_push(m, sizeof(type)))

