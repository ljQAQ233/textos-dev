#include <app/api.h>
#include <app/inet.h>
#include <textos/net.h>
#include <textos/net/ip.h>
#include <textos/net/icmp.h>

#include <stdio.h>

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
    if (inet_aton(argv[1], &((sockaddr_in_t *)&addr)->addr) == 0)
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
