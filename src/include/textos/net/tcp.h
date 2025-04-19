#pragma once

#ifndef CONFIG_BIT_BE

#define TCP_F_FIN (1 << 0)
#define TCP_F_SYN (1 << 1)
#define TCP_F_RST (1 << 2)
#define TCP_F_PSH (1 << 3)
#define TCP_F_ACK (1 << 4)
#define TCP_F_URG (1 << 5)
#define TCP_F_ECE (1 << 6)
#define TCP_F_CWR (1 << 7)

typedef struct _packed
{
    u16 sport;
    u16 dport;
    u32 seqnr;
    u32 acknr;

    u8 rev0 : 4;
    u8 offset : 4; // 4 bit

    union
    {
        struct _packed
        {
            u8 fin : 1;
            u8 syn : 1;
            u8 rst : 1;
            u8 psh : 1;
            u8 ack : 1;
            u8 urg : 1;
            u8 ece : 1;
            u8 cwr : 1;
        };
        u8 flgs;
    };

    u16 window;
    u16 cksum;
    u16 urgptr;
    u32 options[0];
} tcphdr_t;

#else

typedef struct _packed
{
    u16 sport;
    u16 dport;
    u32 seqnr;
    u32 acknr;

    u8 offset : 4; // 4 bit
    u8 rev0 : 4;

    struct _packed
    {
        u8 cwr : 1;
        u8 ece : 1;
        u8 urg : 1;
        u8 ack : 1;
        u8 psh : 1;
        u8 rst : 1;
        u8 syn : 1;
        u8 fin : 1;
    } flags;

    u16 window;
    u16 cksum;
    u16 urgptr;
    u32 options[0];
} tcphdr_t;

#endif

typedef struct _packed
{
    ipv4_t sip;
    ipv4_t dip;
    u8 zero;
    u8 pctl;
    u16 len;
} tcpchk_t;

#include <textos/net/ip.h>

typedef struct
{
    u32 seqnr;
    u32 seqlen;
    ipv4_t sip;
    ipv4_t dip;
    nif_t *nif;
    void *data;
    mbuf_t *buf;
    iphdr_t *iph;
    tcphdr_t *hdr;
} tcpseg_t;
