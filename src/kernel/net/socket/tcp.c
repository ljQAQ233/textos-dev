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
#include <textos/ktimer.h>

#include <string.h>

#include "inter.h"

enum state
{
    CLOSED = 0,
    SYN_SENT,
    SYN_RCVD,
    LISTEN,
    ESTABLISHED,
    CLOSE_WAIT,
    LAST_ACK,
};

static list_t list_common = LIST_INIT(list_common);
static list_t list_listen = LIST_INIT(list_listen);

#define MF_PSH (1 << 0) // PSH set, inform upper-layer immediately
#define LIST_MF(x) (CR(x, mbuf_t, list))

typedef struct
{
    socket_t *sock;
    ipv4_t laddr;
    ipv4_t raddr;
    u16 lport;
    u16 rport;
    int mss;
    bool nagle;  // nagle enabled
    
    int state;
    int errno;
    bool passive;// listen mode
    void *ptcp;  // parent
    list_t pchd; // children
    list_t acpt; // wait for `accept`

    u32 snd_iss;
    u32 snd_una; // smallest seqnr not yet acknowledged by the receiver
    u32 snd_nxt; // next sequence number to be sent
    u32 snd_wnd;
    u32 snd_wl1;
    u32 snd_wl2;
    list_t snd_que;
    list_t una_que;
    list_t buf_que;
    ktimer_t tmr_ack;

    u32 rcv_nxt;
    u32 rcv_irs;

    int rx_waiter;
    int syn_waiter;

    /* TODO : replace nif0 */
} tcp_t;

#define TCP(x) ((tcp_t *)x)

#define DEFMSS 536

#define MAX_PORT 65536

#define TRY_UNBLK(x)         \
    do {                     \
        if (x >= 0)          \
            task_unblock(x); \
        x = -1;              \
    } while (0);

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

static void tcp_do_xmit(tcp_t *tcp);
static void tcp_tx_setup(tcp_t *tcp);
static void tcp_tx_seg(tcp_t *tcp, mbuf_t *m);
static void tcp_tx_ack(tcp_t *tcp);
static void tcp_tx_dly(tcp_t *tcp);
static void tcp_rm_dly(tcp_t *tcp);

static int tcp_socket(socket_t *s)
{
    ASSERTK(s->domain == AF_INET);
    ASSERTK(s->type == SOCK_STREAM);

    tcp_t *t;
    t = s->pri = malloc(sizeof(tcp_t));
    t->sock = s;
    t->lport = 0;
    t->rport = 0;
    memset(t->laddr, 0, sizeof(ipv4_t));
    memset(t->raddr, 0, sizeof(ipv4_t));
    t->mss = DEFMSS;
    t->nagle = true;
    t->state = CLOSED;
    t->errno = 0;
    t->passive = false;
    t->ptcp = NULL;
    t->rx_waiter = -1;
    t->syn_waiter = -1;
    list_init(&t->pchd);
    list_init(&t->acpt);
    list_init(&t->snd_que);
    list_init(&t->una_que);
    list_init(&t->buf_que);
    ktimer_init(&t->tmr_ack);

    list_push(&list_common, &s->intype);

    return 0;
}

static int tcp_bind(socket_t *s, sockaddr_t *addr, size_t len)
{
    if (!addr)
        return -EDESTADDRREQ;

    tcp_t *t = TCP(s->pri);
    sockaddr_in_t *in = (sockaddr_in_t *)addr;
        
    int port = ntohs(in->port);
    if (port)
    {
        if (bitmap_test(&bmp, port))
            return -EADDRINUSE;
        t->lport = port;
    }
    else
    {
        ck_lport(t);
    }

    if (ip_addr_isany(in->addr))
        ip_addr_copy(t->laddr, nif0->ip);
    else
        ip_addr_copy(t->laddr, in->addr);

    return 0;
}

static void block_as(int *as)
{
    // only one task is supported
    ASSERTK(*as == -1);

    *as = task_current()->pid;
    task_block();
    *as = -1;
}

// 一直处于 LISTEN
static int tcp_listen(socket_t *s, int backlog)
{
    tcp_t *t = TCP(s->pri);
    t->state = LISTEN;
    list_remove(&t->sock->intype);
    list_insert(&list_listen, &t->sock->intype);
    return 0;
}

#include <irq.h>

// nonblock mode unsupported
static int tcp_accept(socket_t *s, sockaddr_t *addr, size_t *len)
{
    tcp_t *t = TCP(s->pri);
    tcp_t *conn;

    list_t *ptr;
    UNINTR_AREA({
        if (list_empty(&t->acpt))
            block_as(&t->syn_waiter);
        ptr = list_pop(&t->acpt);
    });

    conn = CR(ptr, tcp_t, acpt);
    ASSERTK(conn->state == ESTABLISHED);

    // truncate
    if (addr && len)
    {
        sockaddr_in_t tmp;
        sockaddr_in_t *in = (sockaddr_in_t *)addr;
        tmp.port = t->rport;
        tmp.family = AF_INET;
        ip_addr_copy(tmp.addr, conn->raddr);
        *len = MIN(*len, sizeof(sockaddr_in_t));
        memcpy(addr, &tmp, *len);
    }
    else if (addr && !len)
        return -EINVAL;
    else if (!addr && len)
        return -EINVAL;

    int fd = socket_makefd(conn->sock);
    if (fd < 0)
    {
        free(t);
        free(conn->sock);
    }
    return fd;
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
    tcp_tx_setup(t);

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

/*
 * nagle's algorithm
 * - 1) 若发送方有未确认数据，延迟发送新的小包，等待 ACK 或数据累计
 * - 2) 不影响大包或立即可发送的数据（如窗口允许 && 无未确认数据）
 */
static void tcp_makeseg(tcp_t *tcp, void *data, size_t len)
{
    size_t seglen = tcp->mss;
    while (len > 0)
    {
        mbuf_t *m = NULL;
        /*
         * nagle 启用, 且还有没有被确认的数据, 合并到上一次的 mbuf
         */
        if (tcp->nagle)
            if (!list_empty(&tcp->snd_que))
                m = LIST_MF(&tcp->snd_que.prev);

        if (!m || m->dlen >= seglen)
        {
            m = mbuf_alloc(MBUF_DEFROOM);
            list_insert(&tcp->snd_que, &m->list);
        }
        
        size_t cpy;
        cpy = MIN(len, seglen);
        cpy = MIN(cpy, seglen - m->dlen);
        memcpy(mbuf_put(m, cpy), data, cpy);

        len -= cpy;
        data += cpy;
    }
}

static ssize_t tcp_sendmsg(socket_t *s, msghdr_t *msg, int flags)
{
    tcp_t *tcp = TCP(s->pri);
    void *data = msg->iov[0].base;
    size_t len = msg->iov[0].len;

    tcp_makeseg(tcp, data, len);
    tcp_do_xmit(tcp);
}

#include <irq.h>

// FIXME: the ptr poped may not be read completely, we should put it back
//        if there's still data remained which would be processed next time.
//        that's what tcp differs from others...
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
        mbuf_t *m = LIST_MF(ptr);
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

static void tcp_tx_setup(tcp_t *tcp)
{
    u32 iss = __ktick ^ 0x20250403;
    tcp->snd_iss = iss;
    tcp->snd_una = iss;
    tcp->snd_nxt = iss + 1;
    tcp->state = SYN_SENT;

    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    net_tx_tcp(nif0, m,
        tcp->raddr, tcp->lport, tcp->rport,
        iss, 0,
        TCP_F_SYN, TCP_WINDOW, 0);
}

static void tcp_tx_agree(tcp_t *tcp)
{
    u32 iss = __ktick ^ 0x20250403;
    tcp->snd_iss = iss;
    tcp->snd_una = iss;
    tcp->snd_nxt = iss + 1;
    tcp->state = SYN_RCVD;

    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    net_tx_tcp(nif0, m,
        tcp->raddr, tcp->lport, tcp->rport,
        iss, tcp->rcv_nxt,
        TCP_F_SYN | TCP_F_ACK, TCP_WINDOW, 0);
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

    // mbuf_free(m);
    return 0;
}

int tcp_exit_with(tcp_t *tcp, int errno)
{
    tcp->errno = errno;
    TRY_UNBLK(tcp->rx_waiter);
    TRY_UNBLK(tcp->syn_waiter);
    
    return errno;
}

#include <textos/assert.h>

#define TCP_SEQ_LT(a, b) ((int32)(a - b) < 0)  // less than
#define TCP_SEQ_LE(a, b) ((int32)(a - b) <= 0) // less or equal

// 执行传输
static void tcp_do_xmit(tcp_t *tcp)
{
    if (tcp->nagle)
        if (tcp->snd_una < tcp->snd_nxt - 1)
            return ;

    tcp_rm_dly(tcp);
    while (!list_empty(&tcp->snd_que))
    {
        list_t *ptr = list_pop(&tcp->snd_que);
        mbuf_t *m = LIST_MF(ptr);
        m->id = tcp->snd_nxt;
        tcp_tx_seg(tcp, m);
        tcp->snd_nxt += m->dlen;
        list_insert_after(&tcp->una_que, ptr);

        if (tcp->nagle)
            return ;
    }
}

// una_que 队列首个字节对应 tcp->snd_una
void tcp_x_ack(tcp_t *tcp, tcpseg_t *seg)
{
    ASSERTK(seg->hdr->ack == true);

    if (TCP_SEQ_LE(seg->hdr->acknr, tcp->snd_una))
    {
        // duplicate ACKs can be ignored
        return ;
    }
    else if (TCP_SEQ_LT(tcp->snd_nxt, seg->hdr->acknr))
    {
        // acks something not yet sent
        tcp_tx_ack(tcp);
        return ;
    }

    // una < ack <= nxt, 可被接收
    // remove them from retransmission queue
    u32 curr = tcp->snd_una;
    list_t *ptr = tcp->una_que.next;
    list_t *nxt = ptr->next;
    for ( ; ptr != &tcp->una_que ; ptr = nxt, nxt = nxt->next)
    {
        mbuf_t *m = LIST_MF(ptr);
        if (m->id < seg->hdr->acknr)
        {
            list_remove(ptr);
            // mbuf_free(m);
        }
    }

    tcp->snd_una = seg->hdr->acknr;

    // send data blocked by nagle
    tcp_do_xmit(tcp);
}

static void tcp_tx_seg(tcp_t *tcp, mbuf_t *m)
{
    net_tx_tcp(nif0, m,
        tcp->raddr, tcp->lport, tcp->rport,
        tcp->snd_nxt, tcp->rcv_nxt,
        TCP_F_ACK, TCP_WINDOW, 0);
}

static void tcp_tx_fin(tcp_t *tcp)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    net_tx_tcp(nif0, m,
        tcp->raddr, tcp->lport, tcp->rport,
        tcp->snd_nxt, tcp->rcv_nxt,
        TCP_F_ACK | TCP_F_FIN, TCP_WINDOW, 0);
    // mbuf_free(m);
}

static void tcp_tx_ack(tcp_t *tcp)
{
    u8 flgs = TCP_F_ACK;
    u32 seqnr = tcp->snd_nxt;
    u32 acknr = tcp->rcv_nxt;
    if (tcp->state == SYN_RCVD)
    {
        flgs |= TCP_F_SYN;
        seqnr = tcp->snd_iss;
    }

    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    net_tx_tcp(nif0, m,
        tcp->raddr, tcp->lport, tcp->rport,
        seqnr, acknr,
        flgs, TCP_WINDOW, 0);
    // mbuf_free(m);
}

static void tcp_tmr_dly(void *arg)
{
    tcp_tx_ack(arg);
}

/*
 * 延时发送 ack
 * 给个缓冲的时间, 看接下来有没有数据要发送, 如果有就可以直接捎带 ack 了
 */
static void tcp_tx_dly(tcp_t *tcp)
{
    ktimer(&tcp->tmr_ack, tcp_tmr_dly, tcp, 50);
}

static void tcp_rm_dly(tcp_t *tcp)
{
    ktimer_kill(&tcp->tmr_ack);
}

static void tcp_go_dly(tcp_t *tcp)
{
    ktimer_fire(&tcp->tmr_ack);
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
            tcp_tx_ack(tcp);
            TRY_UNBLK(tcp->syn_waiter);
        }
        else
        {
            // 没有确认 -> 同时打开
            tcp->state = SYN_RCVD;
            tcp_tx_ack(tcp);
        }
    }

    return 0;
}

// handle state `SYN_RCVD`
int tcp_rx_rcvd(tcp_t *tcp, tcpseg_t *seg)
{
    tcphdr_t *hdr = seg->hdr;
    if (hdr->rst)
    {
        if (tcp->passive)
        {
            // TODO: free
            list_remove(&tcp->pchd);
            list_remove(&tcp->sock->intype);
            return -1;
        }
        return tcp_exit_with(tcp, -ECONNREFUSED);
    }

    // Security check skipped

    if (hdr->syn)
    {
        // return to LISTEN state
        if (tcp->passive)
        {
            // TODO: free
            list_remove(&tcp->pchd);
            list_remove(&tcp->sock->intype);
            return -1;
        }
    }
    
    if (hdr->ack)
    {
        if (TCP_SEQ_LT(tcp->snd_una, hdr->acknr)
            && TCP_SEQ_LE(hdr->acknr, tcp->snd_nxt)) {
            tcp->state = ESTABLISHED; 
            if (tcp->ptcp)
            {
                list_insert(&TCP(tcp->ptcp)->acpt, &tcp->acpt);
                TRY_UNBLK(TCP(tcp->ptcp)->syn_waiter);
            }
            else
            {
                TRY_UNBLK(tcp->syn_waiter);
            }
        } else {
            tcp_tx_reset(seg);
        }
    }

    return 0;
}

// TODO : sort
int tcp_rx_data(tcp_t *tcp, tcpseg_t *seg)
{
    if (tcp->rcv_nxt < seg->seqnr)
    {
        DEBUGK(K_NET, "tcp fast-rexmit applied - %#08x\n", tcp->rcv_nxt);
        tcp_tx_ack(tcp);
        return 1;
    }
    else if (tcp->rcv_nxt > seg->seqnr)
    {
        return 1;
    }
    tcp->rcv_nxt += seg->seqlen;
    tcp_tx_dly(tcp);

    if (!seg->buf->len)
        return 0;
    
    mbuf_pushhdr(seg->buf, tcphdr_t);
    mbuf_pushhdr(seg->buf, iphdr_t);
    list_insert_before(&tcp->buf_que, &seg->buf->list);
    if (seg->hdr->psh)
        seg->buf->flgs = MF_PSH;
    TRY_UNBLK(tcp->rx_waiter);
    return 0;
}

int tcp_rx_listen(tcp_t *tcp, tcpseg_t *seg)
{
    // Any reset is invalid
    // Any acknowledgement is invalid.
    tcphdr_t *hdr = seg->hdr;
    if (hdr->rst)
        return -1;
    if (hdr->ack)
    {
        tcp_tx_reset(seg);
        return -1;
    }

    if (hdr->syn)
    {
        // TODO: backlog / backcnt
        socket_t *sock = malloc(sizeof(socket_t));
        sock->domain = tcp->sock->domain;
        sock->type = tcp->sock->type;
        sock->proto = tcp->sock->proto;
        sock->socktype = tcp->sock->socktype;
        sock->op = tcp->sock->op;
        sock->nif = tcp->sock->nif;

        tcp_t *conn = malloc(sizeof(tcp_t));
        conn->sock = sock;
        sock->pri = conn;
        ip_addr_copy(conn->laddr, tcp->laddr);
        ip_addr_copy(conn->raddr, seg->iph->sip);
        conn->lport = tcp->lport;
        conn->rport = hdr->sport;
        conn->mss = tcp->mss;
        conn->nagle = true;
        conn->passive = true;
        conn->ptcp = tcp;
        conn->state = LISTEN;
        conn->errno = 0;
        list_init(&conn->snd_que);
        list_init(&conn->una_que);
        list_init(&conn->buf_que);
        ktimer_init(&conn->tmr_ack);
        conn->rx_waiter = -1;
        conn->syn_waiter = -1;
        list_insert(&tcp->pchd, &conn->pchd);
        list_insert(&list_common, &conn->sock->intype);

        conn->rcv_nxt = seg->seqnr + 1;
        conn->rcv_irs = seg->seqnr;
        tcp_tx_agree(conn);
    }
}

int tcp_rx_established(tcp_t *tcp, tcpseg_t *seg)
{
    tcphdr_t *hdr = seg->hdr;
    if (hdr->rst)
    {
        return tcp_exit_with(tcp, -ECONNREFUSED);
    }

    if (hdr->syn)
    {
        // issue a challenge ACK & drop
        tcp_tx_ack(tcp);
        return 0;
    }

    if (hdr->ack)
    {
        tcp_x_ack(tcp, seg);
    }

    ASSERTK(hdr->urg == false);
    tcp_rx_data(tcp, seg);

    if (hdr->fin)
    {
        tcp->state = CLOSE_WAIT;
        tcp_go_dly(tcp);
        tcp_tx_fin(tcp);
        tcp->state = LAST_ACK;
    }

    return 0;
}

// 简化版
int tcp_rx_lastack(tcp_t *tcp, tcpseg_t *seg)
{
    tcp->state = CLOSED;
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
    LIST_FOREACH(ptr, &list_common)
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

    LIST_FOREACH(ptr, &list_listen)
    {
        socket_t *s = CR(ptr, socket_t, intype);
        tcp_t *t = TCP(s->pri);
        if (!ip_addr_isany(t->laddr) && !ip_addr_cmp(t->laddr, ip->dip))
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
    seg.iph = ip;
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

        case LISTEN:
            tcp_rx_listen(t, &seg);
            break;

        case ESTABLISHED:
            tcp_rx_established(t, &seg);
            break;

        case LAST_ACK:
            tcp_rx_lastack(t, &seg);
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
    tcp_listen,
    tcp_accept,
    tcp_connect,
    tcp_getsockname,
    tcp_getpeername,
    tcp_sendmsg,
    tcp_recvmsg,
};

#include "inter.h"

void sock_tcp_init()
{
    bitmap_init(&bmp, NULL, MAX_PORT);
    bitmap_set(&bmp, 0);
    sockop_set(SOCK_T_TCP, &op);
}

