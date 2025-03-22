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
    /*
     * in this case we only use a network interface (nic0) to
     * communicate, when there're more cards, laddr is useful.
     */
    ipv4_t laddr;
    ipv4_t raddr;
} raw_t;

static list_t intype = LIST_INIT(intype);

// if it doesn't have a iphdr, kernel will build one
static bool hdrincl(socket_t *s)
{
    raw_t *r = s->pri;
    return r->hdrincl
        || s->proto == IPPROTO_RAW;
}

static bool is_any(ipv4_t ip)
{
    return *((u32 *)ip) == 0;
}

static bool match(ipv4_t a, ipv4_t b)
{
    return *(u32 *)a == *(u32 *)b;
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

    list_push(&intype, &s->intype);
    return 0;
}

static int raw_bind(socket_t *s, sockaddr_t *addr, size_t len)
{
    if (!addr)
        return -EDESTADDRREQ;

    raw_t *r = s->pri;
    sockaddr_in_t *in = (sockaddr_in_t *)addr;

    if (is_any(in->addr))
        memcpy(r->laddr, nic0->ip, sizeof(ipv4_t));
    else
        memcpy(r->laddr, in->addr, sizeof(ipv4_t));

    return 0;
}


static int raw_connect(socket_t *s, sockaddr_t *addr, size_t len)
{
    raw_t *r = s->pri;
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    memcpy(r->raddr, in->addr, sizeof(ipv4_t));
    return 0;
}

static int raw_getsockname(socket_t *s, sockaddr_t *addr, size_t len)
{
    raw_t *r = s->pri;
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    memcpy(in->addr, r->laddr, sizeof(ipv4_t));
    in->family = AF_INET;
    in->port = 0;
}

static int raw_getpeername(socket_t *s, sockaddr_t *addr, size_t len)
{
    raw_t *r = s->pri;
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    memcpy(in->addr, r->raddr, sizeof(ipv4_t));
    in->family = AF_INET;
    in->port = 0;
}

static ssize_t raw_sendmsg(socket_t *s, msghdr_t *msg, int flags)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    void *data = msg->iov[0].base;
    size_t len = msg->iov[0].len;

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
        net_tx_ip(nic0, m, in->addr, s->proto);
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
    list_t *ptr;

    UNINTR_AREA({
        // wait for input
        if (list_empty(&s->rx_queue))
        {
            block_as(&s->rx_waiter);
        }
        ptr = list_pop(&s->rx_queue);
    });

    mbuf_t *m = CR(ptr, mbuf_t, list);
    int len = MIN(msg->iov[0].len, m->len);
    memcpy(msg->iov[0].base, m->head, len);

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

        raw_t *r = s->pri;
        if (!is_any(r->raddr) && !match(r->raddr, ip->sip))
            continue;
        if (!is_any(r->laddr) && !match(r->laddr, ip->dip))
            continue;
        
        mbuf_pushhdr(m, iphdr_t);

        list_push(&s->rx_queue, &m->list);
        if (s->rx_waiter >= 0)
        {
            task_unblock(s->rx_waiter);
            s->rx_waiter = -1;
        }

        ret = 1;
        break;
    }
    
    // XXX: we assume m is constant
    return ret;
}

static sockop_t op = {
    .socket = raw_socket,
    .bind = raw_bind,
    .connect = raw_connect,
    .getsockname = raw_getsockname,
    .getpeername = raw_getpeername,
    .sendmsg = raw_sendmsg,
    .recvmsg = raw_recvmsg,
};

void sock_raw_init()
{
    sockop_set(SOCK_T_RAW, &op);
}

