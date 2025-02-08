#pragma

typedef struct
{
    u8 type;
    u8 code;
    u16 cksum;
    u16 id;
    u16 seq;
    u8 data[0];
} _packed icmphdr_t;

void icmp_request(ipv4_t dip);
