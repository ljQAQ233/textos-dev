/*
 * TODO: fuck up the retval :)
 */
#include <textos/mm.h>
#include <textos/task.h>
#include <textos/errno.h>
#include <textos/net.h>
#include <textos/net/ip.h>
#include <textos/net/udp.h>
#include <textos/net/socket.h>
#include <textos/klib/bitmap.h>
#include <string.h>

#include "inter.h"
#include <textos/panic.h>
#include <textos/assert.h>

#define MAX_PORT 65536

static bitmap_t bmp;
static list_t intype = LIST_INIT(intype);

typedef struct
{
    ipv4_t laddr; // for bind
    ipv4_t raddr; // for send

    u16 lport; // local
    u16 rport; // remote

    list_t rx_que;
    int rx_waiter;
} udp_t;

#define UDP(x) ((udp_t *)x)

static void ck_lport(udp_t *u)
{
    if (u->lport)
        return ;

    int port = bitmap_find(&bmp);
    if (port == -1)
        PANIC("no port can be used\n");
    u->lport = port;
}

static int udp_socket(socket_t *s)
{
    ASSERTK(s->domain == AF_INET);
    ASSERTK(s->type == SOCK_DGRAM);

    udp_t *u;
    u = s->pri = malloc(sizeof(udp_t));
    u->lport = 0;
    u->rport = 0;
    memset(u->laddr, 0, sizeof(ipv4_t));
    memset(u->raddr, 0, sizeof(ipv4_t));
    u->rx_waiter = -1;
    list_init(&u->rx_que);

    list_push(&intype, &s->intype);
    return 0;
}

static int udp_bind(socket_t *s, sockaddr_t *addr, size_t len)
{
    if (!addr)
        return -EDESTADDRREQ;

    udp_t *u = UDP(s->pri);
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
        
    int port = ntohs(in->port);
    if (port)
    {
        if (bitmap_test(&bmp, port))
            return -EADDRINUSE;
    }
    else
    {
        ck_lport(u);
    }

    if (ip_addr_isany(in->addr))
        ip_addr_copy(u->laddr, nif0->ip);
    else
        ip_addr_copy(u->laddr, in->addr);

    return 0;
}

static int udp_connect(socket_t *s, sockaddr_t *addr, size_t len)
{
    udp_t *u = UDP(s->pri);
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    u->rport = ntohs(in->port);
    ip_addr_copy(u->raddr, in->addr);
    return 0;
}

static int udp_getsockname(socket_t *s, sockaddr_t *addr, size_t len)
{
    udp_t *u = UDP(s->pri);
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    ip_addr_copy(in->addr, u->laddr);
    in->family = AF_INET;
    in->port = htons(u->lport);
    return 0;
}

static int udp_getpeername(socket_t *s, sockaddr_t *addr, size_t len)
{
    udp_t *u = UDP(s->pri);
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    ip_addr_copy(in->addr, u->raddr);
    in->family = AF_INET;
    in->port = htons(u->rport);
    return 0;
}

static ssize_t udp_sendmsg(socket_t *s, msghdr_t *msg, int flags)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    void *data = msg->iov[0].base;
    size_t len = msg->iov[0].len;

    void *payload = mbuf_put(m, len);
    memcpy(payload, data, len);

    /*
     * 已经 connect, msg 存在, EISCONN
     * 没有 connect, 使用 msg 提供的
     */
    udp_t *u = UDP(s->pri);
    sockaddr_in_t rdef = {
        .family = AF_INET,
        .port = htons(u->rport),
        .addr = {
            u->raddr[0],
            u->raddr[1],
            u->raddr[2],
            u->raddr[3],
        }
    };
    sockaddr_in_t *in = (sockaddr_in_t *)msg->name;
    if (!ip_addr_isany(u->raddr))
    {
        if (in)
            return -EISCONN;
        in = &rdef;
    }

    if (!in)
        return -EDESTADDRREQ;
    if (!in->port)
        return -EINVAL;
    if (ip_addr_isany(in->addr))
        return -EINVAL;

    ck_lport(u);

    net_tx_udp(nif0, m, in->addr, u->lport, ntohs(in->port));
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

static ssize_t udp_recvmsg(socket_t *s, msghdr_t *msg, int flags)
{
    udp_t *u = UDP(s->pri);
    ck_lport(u);

    list_t *ptr;
    UNINTR_AREA({
        // wait for input
        if (list_empty(&u->rx_que))
        {
            block_as(&u->rx_waiter);
        }
        ptr = list_pop(&u->rx_que);
    });

    mbuf_t *m = CR(ptr, mbuf_t, list);
    iphdr_t *ip = mbuf_pullhdr(m, iphdr_t);
    udphdr_t *hdr = mbuf_pullhdr(m, udphdr_t);
    sockaddr_in_t *in = (sockaddr_in_t *)msg->name;
    // sender's ip and udp port
    if (in)
    {
        ip_addr_copy(in->addr, ip->sip);
        in->family = AF_INET;
        in->port = hdr->sport;
    }
    
    int len = MIN(msg->iov[0].len, m->len);
    memcpy(msg->iov[0].base, m->head, len);

    return len;
}

#include <textos/net/udp.h>

int sock_rx_udp(iphdr_t *ip, mbuf_t *m)
{
    int ret = 0;
    list_t *ptr;
    udphdr_t *hdr;

    hdr = mbuf_pullhdr(m, udphdr_t);
    LIST_FOREACH(ptr, &intype)
    {
        socket_t *s = CR(ptr, socket_t, intype);
        udp_t *u = UDP(s->pri);
        if (!ip_addr_isany(u->raddr))
        {
            if (u->rport != ntohs(hdr->sport))
                continue;
            if (u->lport != ntohs(hdr->dport))
                continue;
        }
        if (!ip_addr_isany(u->raddr) && !ip_addr_cmp(u->raddr, ip->sip))
            continue;
        if (!ip_addr_isany(u->laddr) && !ip_addr_cmp(u->laddr, ip->dip))
            continue;

        mbuf_pushhdr(m, udphdr_t);
        mbuf_pushhdr(m, iphdr_t);

        list_push(&u->rx_que, &m->list);
        if (u->rx_waiter >= 0)
        {
            task_unblock(u->rx_waiter);
            u->rx_waiter = -1;
        }

        ret = 1;
        break;
    }
    
    return ret;
}

static sockop_t op = {
    udp_socket,
    udp_bind,
    noopt,
    noopt,
    udp_connect,
    noopt,
    udp_getsockname,
    udp_getpeername,
    udp_sendmsg,
    udp_recvmsg,
};

void sock_udp_init()
{
    bitmap_init(&bmp, NULL, MAX_PORT / mpembs);
    bitmap_set(&bmp, 0);
    sockop_set(SOCK_T_UDP, &op);
}
