#include <textos/net.h>

void nif_eth_rx(nif_t *n, mbuf_t *m)
{
    ethhdr_t *hdr = mbuf_pullhdr(m, ethhdr_t);

    u16 type = ntohs(hdr->type);
    if (type == ETH_IP)
        net_rx_ip(n, m);
    else if (type == ETH_ARP)
        net_rx_arp(n, m);
}

#include <string.h>

void nif_eth_tx(nif_t *n, mbuf_t *m, mac_t dst, u16 type)
{
    ethhdr_t *hdr = mbuf_pushhdr(m, ethhdr_t);
    eth_addr_copy(hdr->dest, dst);
    eth_addr_copy(hdr->src, n->mac);
    hdr->type = htons(type);

    n->send(n, m);
}
