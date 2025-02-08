#pragma once

#ifndef CONFIG_BYTE_BE

typedef struct
{
    u8 ihl : 4;    // internet hdr len (dword), sizeof(iphdr_t)=20. the minimum is 20/4
    u8 ver : 4;    // ipv4 = 4
    u8 tos;        // type of service
    u16 len;       // ip packet length
    u16 id;        // 
    u16 off0 : 5;  // fragment offset high
    u16 flgs : 3;  // DF / MF
    u8 off1;       // fragment offset low
    u8 ttl;        // time to live
    u8 ptype;      // procotol
    u16 cksum;     // hdr checksum
    ipv4_t sip;    // src ip
    ipv4_t dip;    // dst ip
    u8 opts[0];    // options
    u8 data[0];    // payload
} _packed iphdr_t;

#else

typedef struct
{
    u8 ver : 4;    // ipv4 = 4
    u8 ihl : 4;    // internet hdr len (dword), sizeof(iphdr_t)=20. the minimum is 20/4
    u8 tos;        // type of service
    u16 len;       // ip packet length
    u16 id;        // 
    u16 flgs : 3;  // DF / MF
    u16 off0 : 5;  // fragment offset high
    u16 off1 : 8;  // fragment offset low
    u8 ttl;        // time to live
    u8 ptype;      // procotol
    u16 cksum;     // hdr checksum
    ipv4_t sip;    // src ip
    ipv4_t dip;    // dst ip
    u8 opts[0];    // options
    u8 data[0];    // payload
} _packed iphdr_t;

#endif

#define IP_TTL 64
