#include <textos/net.h>
#include <textos/net/ip.h>
#include <textos/net/icmp.h>

static u16 cksum(void *data, int len)
{
    u32 s = 0;
    u16 *p = (u16 *)data;

    while (len > 1) {
        s += *p++;
        len -= 2;
    }
    if (len == 1)
        s += *(u8 *)p;

    s = (s >> 16) + (s & 0xFFFF);
    s += (s >> 16);
    return (u16)~s;
}

void net_tx_icmp(nif_t *n, u8 type, ipv4_t dip)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    icmphdr_t *hdr = mbuf_pushhdr(m, icmphdr_t);

    hdr->type = type;
    hdr->code = 0;
    hdr->id = htons(1);
    hdr->seq =  htons(1);
    
    hdr->cksum = 0;
    hdr->cksum = cksum(hdr, m->len);

    net_tx_ip(n, m, dip, IP_P_ICMP);
}

void net_rp_icmp(nif_t *n, mbuf_t *m, ipv4_t dip)
{
    icmphdr_t *hdr = mbuf_pushhdr(m, icmphdr_t);
    hdr->type = ICMP_REPLY;
    hdr->cksum = 0;
    hdr->cksum = cksum(hdr, m->len);
    net_tx_ip(n, m, dip, IP_P_ICMP);
}

void net_rx_icmp(nif_t *n, mbuf_t *m, void *iph)
{
    icmphdr_t *hdr = mbuf_pullhdr(m, icmphdr_t);
    iphdr_t *iphdr = (iphdr_t *)iph;

    switch (hdr->type)
    {
        case ICMP_REPLY:
            DEBUGK(K_NET, "ICMP RX reply!!!\n");
            break;

        case ICMP_REQUEST:
            DEBUGK(K_NET, "ICMP RX request echo\n");
            net_rp_icmp(n, m, iphdr->sip);
            break;
    }
}

void icmp_request(ipv4_t dip)
{
    net_tx_icmp(nif0, ICMP_REQUEST, dip);
}

// TODO
// static bool icmp_bc; // accept broadcast
// static bool icmp_mc; // accept multicast
