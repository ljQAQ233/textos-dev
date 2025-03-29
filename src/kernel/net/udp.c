#include <textos/net.h>
#include <textos/net/udp.h>

void net_tx_udp(nif_t *n, mbuf_t *m, ipv4_t dip, u16 sport, u16 dport)
{
    udphdr_t *hdr = mbuf_pushhdr(m, udphdr_t);

    hdr->sport = htons(sport);
    hdr->dport = htons(dport);
    hdr->ulen = htons(m->len);
    hdr->cksum = 0;     // no cksum

    net_tx_ip(n, m, dip, IP_P_UDP);
}

