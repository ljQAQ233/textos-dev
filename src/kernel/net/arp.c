#include <textos/mm.h>
#include <textos/net.h>
#include <textos/net/arp.h>
#include <textos/tick.h>
#include <textos/panic.h>

#include <string.h>

void net_rx_arp(nif_t *n, mbuf_t *m)
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
        if (ip_addr_cmp(n->ip, hdr->dstpr)) 
            net_tx_arp(n, ARP_OP_REPLY, hdr->srchw, hdr->srcpr);
        arp_set(n, hdr->srcpr, hdr->srchw, ARP_TIMEOUT);
    }
    else if (op == ARP_OP_REPLY)
    {
        DEBUGK(K_INFO, "ARP RX reply - %02x:%02x:%02x:%02x:%02x:%02x\n",
          hdr->srchw[0], hdr->srchw[1], hdr->srchw[2],
          hdr->srchw[3], hdr->srchw[4], hdr->srchw[5]);
        arp_set(n, hdr->srcpr, hdr->srchw, ARP_TIMEOUT);
    }

done:
    mbuf_free(m);
}

// XXX: fix -> unsafe address operation
void net_tx_arp(nif_t *n, u16 op, mac_t dmac, ipv4_t dip)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    arphdr_t *hdr = mbuf_pushhdr(m, arphdr_t);

    hdr->htype = htons(1);      // ethernet
    hdr->ptype = htons(0x800);  // ipv4
    hdr->hlen = ETH_HLEN;       // 6
    hdr->plen = ETH_PLEN;       // 4
    hdr->opcode = htons(op);    // req / reply
    eth_addr_copy(hdr->srchw, n->mac);
    ip_addr_copy(hdr->srcpr, n->ip);
    eth_addr_copy(hdr->dsthw, dmac);
    ip_addr_copy(hdr->dstpr, dip);

    // broadcast arp
    nif_eth_tx(n, m, dmac, ETH_ARP);
}

void net_tx_arpip(nif_t *n, mbuf_t *m, ipv4_t dip)
{
    ipv4_t dip0;
    arpent_t *arp = arp_get(n, dip);
    if (ip_addr_maskcmp(dip, n->ip, n->netmask))
        ip_addr_copy(dip0, dip);
    else
        ip_addr_copy(dip0, n->gateway);

    if (!arp)
    {
        arp = arp_init(dip0);
        list_insert(&n->arps, &arp->arps);
    }

    if (!arp->init || arp->expires < __ktick)
    {
        // arp cache not found, request its mac and
        // re-send it after get the destination mac.
        // insert in the tail
        list_insert_before(&arp->ipque, &m->wque);
        arp_request(dip0);
    }
    else
    {
        // cache used
        nif_eth_tx(n, m, arp->hw, ETH_IP);
    }
}

static mac_t bc = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

void arp_request(ipv4_t dip)
{
    net_tx_arp(nif0, ARP_OP_REQUEST, bc, dip);
}

arpent_t *arp_init(ipv4_t dip)
{
    arpent_t *arp = malloc(sizeof(arpent_t));
    ip_addr_copy(arp->ip, dip);
    arp->init = false;
    list_init(&arp->ipque);

    return arp;
}

arpent_t *arp_get(nif_t *n, ipv4_t ip)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &n->arps)
    {
        arpent_t *arp = CR(ptr, arpent_t, arps);
        if (ip_addr_cmp(ip, arp->ip))
            return arp;
    }

    return NULL;
}

/*
 * we register/set an entry when:
 *   - request arrived
 *   - reply accepted
 * uninterruptable env is required
 */
arpent_t *arp_set(nif_t *n, ipv4_t ip, mac_t hw, size_t ticks)
{
    arpent_t *arp = arp_get(n, ip);
    if (!arp)
    {
        arp = malloc(sizeof(arpent_t));
        ip_addr_copy(arp->ip, ip);
        list_init(&arp->ipque);
    }
    else
    {
        list_remove(&arp->arps);
    }

    arp->init = true;
    arp->ticks = ticks;
    arp->expires = __ktick + ticks;
    eth_addr_copy(arp->hw, hw);

    // do send (pop)
    while (!list_empty(&arp->ipque))
    {
        list_t *ptr = arp->ipque.next;
        mbuf_t *mbuf = CR(ptr, mbuf_t, wque);
        nif_eth_tx(n, mbuf, arp->hw, ETH_IP);
        list_remove(ptr);
    }
    list_insert(&n->arps, &arp->arps);

    DEBUGK(K_INFO, "ARP SET - %d:%d:%d:%d -> %02x:%02x:%02x:%02x:%02x:%02x\n",
      ip[0], ip[1], ip[2], ip[3], hw[0], hw[1], hw[2], hw[3], hw[4], hw[5]);
    return arp;
}
