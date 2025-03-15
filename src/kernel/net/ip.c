#include <textos/net.h>
#include <textos/net/ip.h>

#include <string.h>

static bool match(ipv4_t a, ipv4_t b)
{
    return *(u32 *)a == *(u32 *)b;
}

u16 cksum(void *data)
{
    u32 s = 0;
    u16 *p = (u16 *)data;
    int len = 20;

    for (int i = 0; i < len / 2; i++)
        s += p[i];

    while (s >> 16)
        s = (s & 0xFFFF) + (s >> 16);

    return ~s;
}

extern int sock_rx_raw(iphdr_t *hdr, mbuf_t *m);
extern int sock_rx_udp(iphdr_t *hdr, mbuf_t *m);

void net_rx_ip(nic_t *n, mbuf_t *m)
{
    iphdr_t *hdr = mbuf_pullhdr(m, iphdr_t);

    if (hdr->ver != 4)
        goto drop;

    if (hdr->ihl != sizeof(*hdr) / 4)
        goto drop;

    // fragmented packet is not supported
    if ((hdr->flgs & 4) || hdr->off0 || hdr->off1)
        goto drop;

    if (!match(n->ip, hdr->dip))
        goto drop;

    int ret = sock_rx_raw(hdr, m);
    if (ret > 0)
        return ;

    switch (hdr->ptype)
    {
        case IP_P_ICMP:
            net_rx_icmp(n, m, hdr);
            break;
        case IP_P_TCP:
            break;
        case IP_P_UDP:
            sock_rx_udp(hdr, m);
            break;
    }

    return ;

drop:
    mbuf_free(m);
}

static mac_t bc = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void net_tx_ip(nic_t *n, mbuf_t *m, ipv4_t dip, u8 ptype)
{
    iphdr_t *hdr = mbuf_pushhdr(m, iphdr_t);
    hdr->ver = 4;
    hdr->ihl = sizeof(*hdr) / 4;
    hdr->off0 = 0;
    hdr->off1 = 0;
    hdr->flgs = 0;
    hdr->ptype = ptype;
    hdr->ttl = IP_TTL;
    hdr->len = htons(m->len);
    memcpy(hdr->dip, dip, sizeof(ipv4_t));
    memcpy(hdr->sip, n->ip, sizeof(ipv4_t));
    
    hdr->cksum = 0;
    hdr->cksum = cksum(hdr);

    // unicast ippkt
    net_tx_arpip(n, m, hdr->dip);
}
