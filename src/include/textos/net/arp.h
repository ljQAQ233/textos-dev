#pragma once

#include <textos/net.h>

typedef struct arp
{
    u16 htype;  // hardware type
    u16 ptype;  // protocol type
    u8  hlen;   // hardware addr len
    u8  plen;   // prootcol addr len
    u16 opcode; // arp opertion code
    u8  srchw[ETH_HLEN]; // src hw addr 
    u8  srcpr[ETH_PLEN]; // src protocol addr
    u8  dsthw[ETH_HLEN]; //
    u8  dstpr[ETH_PLEN]; //
} _packed arphdr_t;

typedef struct
{
    bool init;
    long ticks;
    long expires;
    ipv4_t ip;
    mac_t hw;
    list_t arps;
    list_t ipque;
} arpent_t;

#define ARP_TIMEOUT 6000 // ticks (n * 10ms)

arpent_t *arp_init(ipv4_t dip);

arpent_t *arp_get(nif_t *n, ipv4_t ip);

arpent_t *arp_set(nif_t *n, ipv4_t ip, mac_t hw, size_t ticks);

void arp_request(ipv4_t dip);
