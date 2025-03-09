#pragma once

typedef struct
{
    u16 sport; // src port
    u16 dport; // dst port
    u16 ulen;  // length (hdr + data)
    u16 cksum; // check sum
    u8 data[0];
} udphdr_t;

