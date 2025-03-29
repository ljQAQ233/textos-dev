#ifndef __NET_H__
#define __NET_H__

#define ETH_HLEN 6
#define ETH_PLEN 4

typedef u8 mac_t[ETH_HLEN];
typedef u8 ipv4_t[ETH_PLEN];

#define ETH_BC  "\xff\xff\xff\xff\xff\xff"
#define ETH_ANY "\x00\x00\x00\x00\x00\x00"

#include <textos/dev/pci.h>
#include <textos/dev/mbuf.h>
#include <textos/klib/list.h>

struct nif;
typedef struct nif nif_t;

struct nif
{
    char name[16];
    mac_t mac;
    ipv4_t ip;
    ipv4_t gateway;
    ipv4_t netmask;
    // TODO : apply broadcast
    ipv4_t broadcast;

    bool link;
    list_t arps;
    list_t nifs; // all nifs
    pci_idx_t *pi;
    void (*send)(nif_t *n, mbuf_t *m);
};

extern nif_t *nif0;

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

// addr op
void eth_addr_copy(mac_t addr1, mac_t addr2);
bool eth_addr_cmp(mac_t addr1, mac_t addr2);
bool eth_addr_isany(mac_t addr);
void ip_addr_copy(ipv4_t addr1, ipv4_t addr2);
bool ip_addr_cmp(ipv4_t addr1, ipv4_t addr2);
bool ip_addr_maskcmp(ipv4_t addr1, ipv4_t addr2, ipv4_t mask);
bool ip_addr_isbroadcast(ipv4_t addr, ipv4_t mask);
bool ip_addr_isany(ipv4_t addr);
bool ip_addr_ismulticast(ipv4_t addr);

// packet handle
void nif_eth_rx(nif_t *n, mbuf_t *m);
void nif_eth_tx(nif_t *n, mbuf_t *m, mac_t dst, u16 type);

int nif_ioctl(nif_t *nif, int req, void *argp);
void nif_register(nif_t *nif);
nif_t *nif_find(char *name);

/*
 * protocol operations:
 *  - rx - input
 *  - tx - output
 *  - rp - reply only
 */

#define IP_P_ICMP 1
#define IP_P_TCP  6
#define IP_P_UDP  17

#define IPPROTO_IP   0
#define IPPROTO_RAW  255
#define IPPROTO_ICMP IP_P_ICMP
#define IPPROTO_TCP  IP_P_TCP
#define IPPROTO_UDP  IP_P_UDP

void net_rx_ip(nif_t *n, mbuf_t *m);
void net_tx_ip(nif_t *n, mbuf_t *m, ipv4_t dip, u8 ptype);

#define ICMP_REPLY   0
#define ICMP_REQUEST 8

void net_rx_icmp(nif_t *n, mbuf_t *m, void *iph);
void net_rp_icmp(nif_t *n, mbuf_t *m, ipv4_t dip);
void net_tx_icmp(nif_t *n, u8 type, ipv4_t dip);

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY   2

void net_rx_arp(nif_t *n, mbuf_t *m);
void net_tx_arp(nif_t *n, u16 op, mac_t dmac, ipv4_t dip);
void net_tx_arpip(nif_t *n, mbuf_t *m, ipv4_t dip);

void net_tx_udp(nif_t *n, mbuf_t *m, ipv4_t dip, u16 sport, u16 dport);

#endif
