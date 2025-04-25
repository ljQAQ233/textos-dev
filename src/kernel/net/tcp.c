#include <textos/net.h>
#include <textos/net/tcp.h>

// checksum (pseudo + tcphdr + data)
u16 net_cksum_tcp(void *data, ipv4_t sip, ipv4_t dip, int len)
{
    tcpchk_t chk;
    ip_addr_copy(chk.sip, sip);
    ip_addr_copy(chk.dip, dip);
    chk.zero = 0;
    chk.pctl = IP_P_TCP;
    chk.len = htons(len);
    
    u32 s = 0;
    u16 *p = data;
    while (len > 1) {
        s += *p++;
        len -= 2;
    }
    if (len == 1)
        s += *(u8 *)p;

    p = (u16 *)&chk;
    len = sizeof(chk);
    while (len > 0) {
        s += *p++;
        len -= 2;
    }

    s = (s >> 16) + (s & 0xFFFF);
    s += (s >> 16);
    return (u16)~s;
}

void net_tx_tcp(
    nif_t *n, mbuf_t *m,
    ipv4_t dip, u16 sport, u16 dport,
    u32 seqnr, u32 acknr,
    u8 flgs, u16 window, u16 urgptr
    )
{
    // tcp header building...
    tcphdr_t *hdr = mbuf_pushhdr(m, tcphdr_t);
    hdr->sport = htons(sport);
    hdr->dport = htons(dport);
    hdr->seqnr = htonl(seqnr);
    hdr->acknr = htonl(acknr);
    hdr->rev0 = 0;
    hdr->offset = 5;
    hdr->flgs = flgs;
    hdr->window = htons(window);
    hdr->urgptr = htons(urgptr);

    hdr->cksum = 0;
    hdr->cksum = net_cksum_tcp(m->head, n->ip, dip, m->len);

    // ip output
    net_tx_ip(n, m, dip, IP_P_TCP);
}
