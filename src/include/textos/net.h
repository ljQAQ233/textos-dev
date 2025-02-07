#ifndef __NET_H__
#define __NET_H__

#define ETH_HLEN 6
#define ETH_PLEN 4

typedef u8 mac_t[6];
typedef u8 ipv4_t[4];

#include <textos/dev/pci.h>
#include <textos/dev/mbuf.h>
#include <textos/klib/list.h>

struct nic;
typedef struct nic nic_t;

struct nic
{
    u8 mac[6];
    u8 ip[4];
    bool link;
    list_t nics; // all nics
    pci_idx_t *pi;

    void (*send)(nic_t *n, mbuf_t *m);
};

extern nic_t *nic0;

typedef struct
{
    mac_t dest;
    mac_t src;
    u16 type;
    u8 data[0];
} _packed ethhdr_t;

#define ETH_IP  0x0800 // Internet protocol
#define ETH_ARP 0x0806 // Address resolution protocol

// convert
u16 htons(u16 h);
u16 ntohs(u16 h);
u32 htonl(u32 h);
u32 ntohl(u32 h);

// packet handle
void nic_eth_rx(nic_t *n, mbuf_t *m);
void nic_eth_tx(nic_t *n, mbuf_t *m, mac_t dst, u16 type);

void net_rx_ip(mbuf_t *m);
void net_tx_ip(nic_t *n, mbuf_t *m);

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY   2

void net_rx_arp(nic_t *n, mbuf_t *m);
void net_tx_arp(nic_t *n, u16 op, mac_t dmac, ipv4_t dip);

#endif
