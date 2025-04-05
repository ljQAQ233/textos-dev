/*
 * TCP Implementation
 */

#include <textos/net.h>
#include <textos/net/ip.h>    
#include <textos/net/tcp.h>
#include <textos/net/socket.h>
#include <textos/klib/bitmap.h>
#include <textos/mm.h>
#include <textos/panic.h>
#include <textos/assert.h>
#include <textos/errno.h>

#include <string.h>

enum state
{
    CLOSED = 0,
    SYN_SENT,
    SYN_RCVD,
    ESTABLISHED,
};

static list_t intype = LIST_INIT(intype);
    
#define MF_PSH (1 << 0) // PSH set, inform upper-layer immediately

typedef struct
{
    ipv4_t laddr;
    ipv4_t raddr;
    u16 lport;
    u16 rport;
    int mss;

    int state;
    int errno;
    u32 snd_iss;
    u32 snd_una; // smallest seqnr not yet acknowledged by the receiver
    u32 snd_nxt; // next sequence number to be sent
    u32 snd_wnd;
    u32 snd_wl1;
    u32 snd_wl2;
    list_t snd_que;
    list_t ack_que;
    list_t buf_que;

    u32 rcv_nxt;
    u32 rcv_irs;

    int rx_waiter;
    int syn_waiter;

    /* TODO : replace nif0 */
} tcp_t;

#define TCP(x) ((tcp_t *)x)

#define DEFMSS 536

#define MAX_PORT 65536

static bitmap_t bmp;

static void ck_lport(tcp_t *t)
{
    if (t->lport)
        return ;

    int port = bitmap_find(&bmp);
    if (port == -1)
        PANIC("no port can be used\n");
    t->lport = port;
}

static void tcp_tx_setup(tcp_t *tcp, ipv4_t dip, u16 sport, u16 dport);
static int tcp_tx_xmit(tcp_t *tcp);
static int tcp_tx_ack(tcp_t *tcp, u32 seqnr, u32 acknr, u8 flgs);

static int tcp_socket(socket_t *s)
{
    ASSERTK(s->domain == AF_INET);
    ASSERTK(s->type == SOCK_STREAM);

    tcp_t *t;
    t = s->pri = malloc(sizeof(tcp_t));
    t->lport = 0;
    t->rport = 0;
    t->mss = DEFMSS;
    t->state = CLOSED;
    t->errno = 0;
    t->rx_waiter = -1;
    t->syn_waiter = -1;
    list_init(&t->snd_que);
    list_init(&t->ack_que);
    list_init(&t->buf_que);

    list_insert(&intype, &s->intype);

    return 0;
}

static int tcp_bind(socket_t *s, sockaddr_t *addr, size_t len)
{

}

static void block_as(int *as)
{
    // only one task is supported
    ASSERTK(*as == -1);

    *as = task_current()->pid;
    task_block();
    *as = -1;
}

static int tcp_connect(socket_t *s, sockaddr_t *addr, size_t len)
{
    tcp_t *t = TCP(s->pri);
    // connected
    if (t->rport)
        return -EINVAL;

    sockaddr_in_t *in = (sockaddr_in_t *)addr;
    if (!in->port)
        return -EINVAL;
    ip_addr_copy(t->raddr, in->addr);
    t->rport = ntohs(in->port);

    // allocate local port
    ck_lport(t);
    tcp_tx_setup(t, in->addr, t->lport, t->rport);

    block_as(&t->syn_waiter);
    ASSERTK(t->state == ESTABLISHED);

    return 0;
}

static int tcp_getsockname(socket_t *s, sockaddr_t *addr, size_t len)
{

}

static int tcp_getpeername(socket_t *s, sockaddr_t *addr, size_t len)
{

}

static ssize_t tcp_sendmsg(socket_t *s, msghdr_t *msg, int flags)
{
    tcp_t *tcp = TCP(s->pri);
    void *data = msg->iov[0].base;
    size_t len = msg->iov[0].len;
    size_t mss = tcp->mss;

    while (len > 0)
    {
        size_t cpy = MIN(len, mss);
        mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
        memcpy(mbuf_put(m, cpy), data, cpy);
        list_insert(&tcp->snd_que, &m->list);

        len -= cpy;
        data += cpy;
    }

    tcp_tx_xmit(tcp);
}

#include <irq.h>

static ssize_t tcp_recvmsg(socket_t *s, msghdr_t *msg, int flags)
{
    tcp_t *tcp = TCP(s->pri);
    void *data = msg->iov[0].base;
    size_t len = msg->iov[0].len;
    size_t mss = tcp->mss;
    ssize_t rem = len;

    UNINTR_AREA_START();

    while (len > 0)
    {
        if (list_empty(&tcp->buf_que))
        {
            block_as(&tcp->rx_waiter);
        }
        if (tcp->errno < 0)
            return tcp->errno;
        
        list_t *ptr = tcp->buf_que.next;
        mbuf_t *m = CR(ptr, mbuf_t, list);
        mbuf_pullhdr(m, iphdr_t);
        mbuf_pullhdr(m, tcphdr_t);
        size_t cpy = MIN(rem, m->len);
        memcpy(data, m->head, cpy);
        list_remove(ptr);
        DEBUGK(K_NET, "tcp rcvd seqnr=%u siz=%u psh=%d\n", m->id, cpy, m->flgs & MF_PSH);

        rem -= cpy;
        data += cpy;
        if (m->flgs & MF_PSH)
        {
            break;
        }
    }

    UNINTR_AREA_END();

    return len - rem;
}

//
//
//

#include <textos/tick.h>
#include <textos/klib/bitmap.h>

#define TCP_WINDOW 65535

static void tcp_tx_setup(tcp_t *tcp, ipv4_t dip, u16 sport, u16 dport)
{
    u32 iss = __ktick ^ 0x20250403;
    tcp->snd_iss = iss;
    tcp->snd_una = iss;
    tcp->snd_nxt = iss + 1;
    tcp->state = SYN_SENT;

    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    net_tx_tcp(nif0, m,
        dip, sport, dport,
        iss, 0,
        TCP_F_SYN, TCP_WINDOW, 0);
}

int tcp_tx_reset(tcpseg_t *seg)
{
    tcphdr_t *hdr = seg->hdr;
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    if (hdr->ack)
    {
        net_tx_tcp(seg->nif, m,
            seg->sip, hdr->dport, hdr->sport,
            hdr->acknr, 0,
            TCP_F_RST, 0, 0);
    }
    else
    {
        // ACK bit is off, sequence number zero is used
        net_tx_tcp(seg->nif, m,
            seg->sip, hdr->dport, hdr->sport,
            0, seg->seqnr + seg->seqlen,
            TCP_F_RST | TCP_F_ACK, 0, 0);
    }

    mbuf_free(m);
    return 0;
}

int tcp_exit_with(tcp_t *tcp, int errno)
{
    tcp->errno = errno;
    if (tcp->rx_waiter >= 0)
        task_unblock(tcp->rx_waiter);
    if (tcp->syn_waiter >= 0)
        task_unblock(tcp->syn_waiter);
    
    return errno;
}

#include <textos/assert.h>

#define TCP_SEQ_LT(a, b) ((int32)(a - b < 0))  // less than
#define TCP_SEQ_LE(a, b) ((int32)(a - b <= 0)) // less or equal

// XXX: 相信对方的流量, 一股脑就发出去了
static int tcp_tx_xmit(tcp_t *tcp)
{
    int ret = 0;
    while (!list_empty(&tcp->snd_que))
    {
        list_t *ptr = list_pop(&tcp->snd_que);
        mbuf_t *m = CR(ptr, mbuf_t, list);
        m->id = tcp->snd_nxt;
        net_tx_tcp(nif0, m,
            tcp->raddr, tcp->lport, tcp->rport,
            tcp->snd_nxt, tcp->rcv_nxt,
            TCP_F_ACK, TCP_WINDOW, 0);
        tcp->snd_nxt += m->dlen;
        list_insert_after(&tcp->ack_que, ptr);
        ret++;
    }

    return ret;
}

// ack_que 队列首个字节对应 tcp->snd_una
int tcp_x_ack(tcp_t *tcp, tcpseg_t *seg)
{
    ASSERTK(seg->hdr->ack == true);

    if (TCP_SEQ_LE(seg->hdr->acknr, tcp->snd_una))
    {
        // duplicate ACKs can be ignored
        return 0;
    }
    else if (TCP_SEQ_LT(tcp->snd_nxt, seg->hdr->acknr))
    {
        // acks something not yet sent
        return tcp_tx_ack(tcp,
                tcp->snd_nxt, tcp->rcv_nxt,
                TCP_F_ACK);
    }

    // una < ack <= nxt, 可被接收
    // remove them from retransmission queue
    u32 curr = tcp->snd_una;
    list_t *ptr = tcp->ack_que.next;
    list_t *nxt = ptr->next;
    for ( ; ptr != &tcp->ack_que ; ptr = nxt, nxt = nxt->next)
    {
        mbuf_t *m = CR(ptr, mbuf_t, list);
        if (m->id < seg->hdr->acknr)
        {
            list_remove(ptr);
            mbuf_free(m);
        }
    }

    tcp->snd_una = seg->hdr->acknr;
    return 0;
}

int tcp_tx_ack(tcp_t *tcp, u32 seqnr, u32 acknr, u8 flgs)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    net_tx_tcp(nif0, m,
        tcp->raddr, tcp->lport, tcp->rport,
        seqnr, acknr,
        flgs, TCP_WINDOW, 0);
    mbuf_free(m);

    return 0;
}

// handle state `SYN_SENT`
int tcp_rx_sent(tcp_t *tcp, tcpseg_t *seg)
{
    tcphdr_t *hdr = seg->hdr;

    // 1. check ACK
    if (hdr->ack)
    {
        if (hdr->acknr != tcp->snd_nxt)
            return tcp_tx_reset(seg);
    }

    // 2. check RST
    // If the ACK was acceptable then signal the user "error:
    // connection reset", drop the segment, enter CLOSED state,
    // delete TCB, and return.  Otherwise (no ACK) drop the segment
    // and return.
    if (hdr->rst)
    {
        if (hdr->ack)
        {
            tcp->state = CLOSED;
            tcp_exit_with(tcp, -ECONNRESET);
        }
    }

    // 3. Security check skipped

    // 4. check SYN
    // 两种情况:
    //  - SYN - 同时打开
    //  - SYN | ACK - 收到对方回复
    if (hdr->syn)
    {
        tcp->rcv_irs = seg->seqnr;     // 初始序号
        tcp->rcv_nxt = seg->seqnr + 1; // SYN 会占用序列号
        if (hdr->ack)
            tcp->snd_una = seg->hdr->acknr;

        // SYN 已经被确认了
        if ((int32)(tcp->snd_una - tcp->snd_iss) > 0)
        {
            tcp->state = ESTABLISHED;
            tcp_tx_ack(tcp,
                tcp->snd_nxt, tcp->rcv_nxt,
                TCP_F_ACK);
            if (tcp->syn_waiter > 0)
                task_unblock(tcp->syn_waiter);
        }
        else
        {
            // 没有确认 -> 同时打开
            tcp->state = SYN_RCVD;
            tcp_tx_ack(tcp,
                tcp->snd_iss, tcp->rcv_nxt,
                TCP_F_ACK | TCP_F_SYN);
        }
    }

    return 0;
}

// handle state `SYN_RCVD`
int tcp_rx_rcvd(tcp_t *tcp, tcpseg_t *seg)
{
    // XXX: 假设现在还不支持 listen

    tcphdr_t *hdr = seg->hdr;
    if (hdr->rst)
    {
        return tcp_exit_with(tcp, -ECONNREFUSED);
    }

    // Security check skipped
    
    if (hdr->syn)
    {
        if (TCP_SEQ_LT(tcp->snd_una, hdr->acknr)
            && TCP_SEQ_LE(hdr->acknr, tcp->snd_nxt)) {
            tcp->state = ESTABLISHED;
            if (tcp->syn_waiter > 0)
                task_unblock(tcp->syn_waiter);
        } else {
            tcp_tx_reset(seg);
        }
    }

    return 0;
}

// TODO : sort
int tcp_rx_data(tcp_t *tcp, tcpseg_t *seg)
{
    if (!seg->buf->len)
        return 0;

    mbuf_pushhdr(seg->buf, tcphdr_t);
    mbuf_pushhdr(seg->buf, iphdr_t);
    if (tcp->rcv_nxt < seg->seqnr)
    {
        DEBUGK(K_NET, "tcp fast-rexmit applied - %#08x\n", tcp->rcv_nxt);
        tcp_tx_ack(tcp,
            tcp->snd_nxt, tcp->rcv_nxt,
            TCP_F_ACK);
        return 1;
    }
    else if (tcp->rcv_nxt > seg->seqnr)
    {
        return 1;
    }
    tcp->rcv_nxt += seg->seqlen;
    tcp_tx_ack(tcp,
        tcp->snd_nxt, tcp->rcv_nxt,
        TCP_F_ACK);
    list_insert_before(&tcp->buf_que, &seg->buf->list);
    if (seg->hdr->psh)
        seg->buf->flgs = MF_PSH;
    if (tcp->rx_waiter > 0)
        task_unblock(tcp->rx_waiter);
    return 0;
}

int tcp_rx_established(tcp_t *tcp, tcpseg_t *seg)
{
    // XXX: 假设现在还不支持 listen

    tcphdr_t *hdr = seg->hdr;
    if (hdr->rst)
    {
        return tcp_exit_with(tcp, -ECONNREFUSED);
    }

    if (hdr->syn)
    {
        // issue a challenge ACK & drop
        tcp_tx_ack(tcp,
            tcp->snd_nxt, tcp->rcv_nxt,
            TCP_F_ACK);
        return 0;
    }

    if (hdr->ack)
    {
        tcp_x_ack(tcp, seg);
    }

    ASSERTK(hdr->urg == false);
    tcp_rx_data(tcp, seg);
    ASSERTK(hdr->fin == false);

    return 0;
}

int tcp_rx_fin1(tcp_t *tcp, tcpseg_t *seg)
{
}

int tcp_rx_fin2(tcp_t *tcp, tcpseg_t *seg)
{

}

int tcp_rx_closing(tcp_t *tcp, tcpseg_t *seg)
{

}

int tcp_rx_timewait(tcp_t *tcp, tcpseg_t *seg)
{

}

static tcp_t *find_tcb(iphdr_t *ip, tcphdr_t *hdr)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &intype)
    {
        socket_t *s = CR(ptr, socket_t, intype);
        tcp_t *t = TCP(s->pri);
        if (!ip_addr_cmp(t->raddr, ip->sip))
            continue;
        if (!ip_addr_isany(t->laddr) && !ip_addr_cmp(t->laddr, ip->dip))
            continue;
        if (t->rport != hdr->sport)
            continue;
        if (t->lport != hdr->dport)
            continue;

        return t;
    }

    return NULL;
}

int sock_rx_tcp(iphdr_t *ip, mbuf_t *m)
{
    if (net_cksum_tcp(m->head, ip->sip, ip->dip, m->len))
        return 0;
    tcphdr_t *hdr = mbuf_pullhdr(m, tcphdr_t);
    hdr->sport = ntohs(hdr->sport);
    hdr->dport = ntohs(hdr->dport);
    hdr->seqnr = ntohl(hdr->seqnr);
    hdr->acknr = ntohl(hdr->acknr);
    hdr->window = ntohs(hdr->window);
    hdr->urgptr = ntohs(hdr->urgptr);

    tcpseg_t seg;
    seg.seqnr = hdr->seqnr;
    seg.seqlen = m->len + hdr->syn + hdr->fin;
    ip_addr_copy(seg.sip, ip->sip);
    ip_addr_copy(seg.dip, ip->dip);
    seg.nif = nif0;
    seg.data = m->head;
    seg.buf = m;
    seg.buf->id = seg.seqnr;
    seg.hdr = hdr;

    tcp_t *t = find_tcb(ip, hdr);
    if (!t)
    {
        tcp_tx_reset(&seg);
        goto done;
    }

    switch (t->state) {
        case SYN_SENT:
            tcp_rx_sent(t, &seg);
            break;

        case SYN_RCVD:
            tcp_rx_rcvd(t, &seg);
            break;

        case ESTABLISHED:
            tcp_rx_established(t, &seg);
            break;

        default:
            tcp_tx_reset(&seg);
            break;
    }

done:
    // TODO : recycle
    // mbuf_free(m);
    return 1;
}

static sockop_t op = {
    tcp_socket,
    tcp_bind,
    tcp_connect,
    tcp_getsockname,
    tcp_getpeername,
    tcp_sendmsg,
    tcp_recvmsg,
};

#include "inter.h"

void sock_tcp_init()
{
    bitmap_init(&bmp, NULL, MAX_PORT / mpembs);
    bitmap_set(&bmp, 0);
    sockop_set(SOCK_T_TCP, &op);
}

