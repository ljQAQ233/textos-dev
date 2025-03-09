#include <app/api.h>
#include <textos/net.h>
#include <textos/net/ip.h>
#include <textos/net/icmp.h>

#include <stdio.h>

int inet_aton(const char *s, sockaddr_in_t *in)
{
    u8 *a = in->addr;
    int p[4], c = 0, v = 0;
    const char *x = s;
    while (*x)
    {
        if ('0' <= *x && *x <= '9') {
            v = v * 10 + (*x - '0');
            if (v > 255)
                return 0;
        } else if (*x == '.') {
            if (c >= 3) return 0;
            p[c++] = v;
            v = 0;
        } else return 0;
        x++;
    }

    if (c != 3) return 0;
    p[c] = v;
    a[0] = p[0];
    a[1] = p[1];
    a[2] = p[2];
    a[3] = p[3];
    return 1;
}

u16 htons(u16 h)
{
    return ((h & 0xFF00) >> 8) | ((h & 0x00FF) << 8);
}

#define ICMP_REPLY   0
#define ICMP_REQUEST 8

static u16 cksum(void *data, int len)
{
    u32 s = 0;
    u16 *p = (u16 *)data;

    while (len > 1) {
        s += *p++;
        len -= 2;
    }
    if (len == 1)
        s += *(u8 *)p;

    s = (s >> 16) + (s & 0xFFFF);
    s += (s >> 16);
    return (u16)~s;
}

void dump(char *buf, ssize_t len)
{
    sockaddr_t sip;
    size_t slen = sizeof(sip);

    iphdr_t *ip = (iphdr_t *)buf;
    icmphdr_t *hdr = (icmphdr_t *)(buf + ip->ihl * 4);

    if (hdr->type == ICMP_REPLY)
    {
        printf("%d bytes from %d.%d.%d.%d: icmp_seq=%d ttl=%d\n",
            len, ip->sip[0], ip->sip[1], ip->sip[2], ip->sip[3],
            hdr->seq, ip->ttl);
    }
}

int main(int argc, char const *argv[])
{
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0)
        goto die;

    sockaddr_t addr;
    if (inet_aton(argv[1], (sockaddr_in_t *)&addr) == 0)
        goto die;
    
    u16 seq = 1;
    char tx_buf[128];
    char rx_buf[128];

    icmphdr_t *hdr = (icmphdr_t *)tx_buf;
    hdr->type = ICMP_REQUEST;
    hdr->code = 0;
    hdr->id = htons(1);
    hdr->seq = htons(seq);
    hdr->cksum = 0;
    hdr->cksum = cksum(hdr, sizeof(icmphdr_t));

    // 8
    int len = sizeof(*hdr);
    while (true)
    {
        int ret = sendto(fd, tx_buf, len, 0, &addr, sizeof(sockaddr_in_t));
        if (ret < 0)
            goto die;

        ret = recvfrom(fd, rx_buf, sizeof(rx_buf), 0, &addr, sizeof(sockaddr_in_t));
        if (ret < 0)
            goto die;

        dump(rx_buf, ret);
    }

    return 0;

die:
    perror("ping");
    return 1;
}
