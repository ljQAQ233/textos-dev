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

