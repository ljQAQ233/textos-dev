#include <textos/net.h>

void nic_eth_rx(nic_t *n, mbuf_t *m)
{
    ethhdr_t *hdr = mbuf_pullhdr(m, ethhdr_t);

    u16 type = ntohs(hdr->type);
    if (type == ETH_ARP)
        net_rx_arp(n, m);
}

#include <string.h>

void nic_eth_tx(nic_t *n, mbuf_t *m, mac_t dst, u16 type)
{
    ethhdr_t *hdr = mbuf_pushhdr(m, ethhdr_t);
    memcpy(hdr->dest, dst, sizeof(mac_t));
    memcpy(hdr->src, n->mac, sizeof(mac_t));
    hdr->type = htons(type);

    n->send(n, m);
}
