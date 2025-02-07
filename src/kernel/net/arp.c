#include <textos/net.h>
#include <textos/net/arp.h>
#include <textos/panic.h>

#include <string.h>

static bool match(ipv4_t *a, ipv4_t *b)
{
    return *(u32 *)a == *(u32 *)b;
}

void net_rx_arp(nic_t *n, mbuf_t *m)
{
    arphdr_t *hdr = mbuf_pullhdr(m, arphdr_t);

    if (ntohs(hdr->htype) != 1 ||
        ntohs(hdr->ptype) != 0x800 ||
        hdr->hlen != ETH_HLEN ||
        hdr->plen != ETH_PLEN)
        goto done;

    u16 op = ntohs(hdr->opcode);
    if (op == ARP_OP_REQUEST)
    {
        if (match(&n->ip, &hdr->dstpr)) 
            net_tx_arp(n, ARP_OP_REPLY, hdr->srchw, hdr->srcpr);
    }
    else if (op == ARP_OP_REPLY)
    {
        DEBUGK(K_LOGK, "ARP RX reply - %02x:%02x:%02x:%02x:%02x:%02x\n",
          hdr->srchw[0], hdr->srchw[1], hdr->srchw[2],
          hdr->srchw[3], hdr->srchw[4], hdr->srchw[5]);
    }

done:
    mbuf_free(m);
}

void net_tx_arp(nic_t *n, u16 op, mac_t dmac, ipv4_t dip)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    arphdr_t *hdr = mbuf_pushhdr(m, arphdr_t);

    hdr->htype = htons(1);      // ethernet
    hdr->ptype = htons(0x800);  // ipv4
    hdr->hlen = ETH_HLEN;       // 6
    hdr->plen = ETH_PLEN;       // 4
    hdr->opcode = htons(op);    // req / reply
    memcpy(hdr->srchw, n->mac, sizeof(mac_t));
    memcpy(hdr->srcpr, n->ip, sizeof(ipv4_t));
    memcpy(hdr->dsthw, dmac, sizeof(mac_t));
    memcpy(hdr->dstpr, dip, sizeof(ipv4_t));

    // broadcast arp
    nic_eth_tx(n, m, dmac, ETH_ARP);
}

void arp_request(ipv4_t dip)
{
    mac_t dmac = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    net_tx_arp(nic0, ARP_OP_REQUEST, dmac, dip);
}
