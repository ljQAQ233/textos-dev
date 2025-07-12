// socket raw provides a interface to operate iphdr directly

#include <textos/mm.h>
#include <textos/task.h>
#include <textos/errno.h>
#include <textos/net.h>
#include <textos/net/ip.h>
#include <textos/net/socket.h>
#include <string.h>

#include "inter.h"
#include <textos/panic.h>
#include <textos/assert.h>

typedef struct
{
    bool hdrincl;
    ipv4_t laddr;
    ipv4_t raddr;
    
    int rx_waiter;
    list_t rx_que;
} raw_t;

#define RAW(x) ((raw_t *)x)

static list_t intype = LIST_INIT(intype);

// if it doesn't have a iphdr, kernel will build one
static bool hdrincl(socket_t *s)
{
    return RAW(s->pri)->hdrincl
        || s->proto == IPPROTO_RAW;
}

static int raw_socket(socket_t *s)
{
    ASSERTK(s->domain == AF_INET);
    ASSERTK(s->type == SOCK_RAW);

    raw_t *r;
    r = s->pri = malloc(sizeof(raw_t));
    r->hdrincl = false; // iphdr not provided by user
    memset(r->laddr, 0, sizeof(r->laddr));
    memset(r->raddr, 0, sizeof(r->raddr));
    r->rx_waiter = -1;
    list_init(&r->rx_que);

    list_pushback(&intype, &s->intype);
    return 0;
}

static int raw_bind(socket_t *s, sockaddr_t *addr, socklen_t len)
{
    if (!addr)
        return -EDESTADDRREQ;

    raw_t *r = RAW(s->pri);
    sockaddr_in_t *in = (sockaddr_in_t *)addr;

    if (ip_addr_isany(in->addr))
        ip_addr_copy(r->laddr, s->nif->ip);
    else
        ip_addr_copy(r->laddr, in->addr);

    return 0;
}


static int raw_connect(socket_t *s, sockaddr_t *addr, socklen_t len)
{
    raw_t *r = RAW(s->pri);
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    ip_addr_copy(r->raddr, in->addr);
    return 0;
}

static int raw_getsockname(socket_t *s, sockaddr_t *addr, socklen_t *len)
{
    raw_t *r = RAW(s->pri);
    sockaddr_in_t in;
    ip_addr_copy(in.addr, r->laddr);
    in.family = AF_INET;
    in.port = 0;
    *len = MIN(*len, sizeof(in));
    memcpy(addr, &in, *len);
    return 0;
}

static int raw_getpeername(socket_t *s, sockaddr_t *addr, socklen_t *len)
{
    raw_t *r = RAW(s->pri);
    sockaddr_in_t in;
    ip_addr_copy(in.addr, r->raddr);
    in.family = AF_INET;
    in.port = 0;
    *len = MIN(*len, sizeof(in));
    memcpy(addr, &in, *len);
    return 0;
}

static ssize_t raw_sendmsg(socket_t *s, msghdr_t *msg, int flags)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    void *data = msg->iov[0].iov_base;
    size_t len = msg->iov[0].iov_len;

    void *payload = mbuf_put(m, len);
    memcpy(payload, data, len);

    sockaddr_in_t *in = (sockaddr_in_t *)msg->name;
    if (hdrincl(s))
    {
        // not supported yet
        PANIC("hdr included!!!\n");
    }
    else
    {
        net_tx_ip(s->nif, m, in->addr, s->proto);
    }
    return len;
}

#include <irq.h>

static void block_as(int *as)
{
    // only one task is supported
    ASSERTK(*as == -1);

    *as = task_current()->pid;
    task_block();
    *as = -1;
}

// TODO: timeout
static ssize_t raw_recvmsg(socket_t *s, msghdr_t *msg, int flags)
{
    raw_t *raw = RAW(s->pri);
    list_t *ptr;

    UNINTR_AREA({
        // wait for input
        if (list_empty(&raw->rx_que))
        {
            block_as(&raw->rx_waiter);
        }
        ptr = list_popback(&raw->rx_que);
    });

    mbuf_t *m = CR(ptr, mbuf_t, list);
    int len = MIN(msg->iov[0].iov_len, m->len);
    memcpy(msg->iov[0].iov_base, m->head, len);
    mbuf_free(m);
    return len;
}

// m includes iphdr
// retval 1 means that m has been handled
int sock_rx_raw(iphdr_t *ip, mbuf_t *m)
{
    int ret = 0;

    list_t *ptr;
    LIST_FOREACH(ptr, &intype)
    {
        socket_t *s = CR(ptr, socket_t, intype);
        if ((u8)s->proto != IPPROTO_RAW)
        {
            if ((u8)s->proto != ip->ptype)
                continue;
        }

        raw_t *r = RAW(s->pri);
        if (!ip_addr_isany(r->raddr) && !ip_addr_cmp(r->raddr, ip->sip))
            continue;
        if (!ip_addr_isany(r->laddr) && !ip_addr_cmp(r->laddr, ip->dip))
            continue;
        
        mbuf_pushhdr(m, iphdr_t);

        list_pushback(&r->rx_que, &m->list);
        if (r->rx_waiter >= 0)
        {
            task_unblock(r->rx_waiter);
            r->rx_waiter = -1;
        }

        ret = 1;
        break;
    }
    
    // XXX: we assume m is constant
    return ret;
}

static sockop_t op = {
    raw_socket,
    raw_bind,
    noopt,
    noopt,
    raw_connect,
    noopt,
    raw_getsockname,
    raw_getpeername,
    raw_sendmsg,
    raw_recvmsg,
};

void sock_raw_init()
{
    sockop_set(SOCK_T_RAW, &op);
}

