#include <textos/textos.h>
#include <textos/net.h>
#include <textos/net/ip.h>
#include <textos/net/icmp.h>

// HACK: define it to prevent including kernel's internal headerfiles
//       DO NOT use kernel's headerfile directly in user program!!!
//       use definitions in <netinet/ip_icmp.h> / <netinet/ip.h> instead!!!
#define __LOCK__

#include <textos/net/socket.h>

#include <stdio.h>

#define ICMP_REPLY   0
#define ICMP_REQUEST 8

static uint16_t cksum(void *data, int len)
{
    uint32_t s = 0;
    uint16_t *p = (uint16_t *)data;

    while (len > 1) {
        s += *p++;
        len -= 2;
    }
    if (len == 1)
        s += *(uint8_t *)p;

    s = (s >> 16) + (s & 0xFFFF);
    s += (s >> 16);
    return (uint16_t)~s;
}

void dump(char *buf, ssize_t len)
{
    sockaddr_in_t sip;
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

extern int inet_aton(const char *s, uint32_t *in);

int main(int argc, char const *argv[])
{
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0)
        goto die;

    sockaddr_in_t addr;
    if (inet_aton(argv[1], (uint32_t *)(&((sockaddr_in_t *)&addr)->addr)) == 0)
        goto die;
    
    uint16_t seq = 1;
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
        int ret = sendto(fd, tx_buf, len, 0, (sockaddr_t *)&addr, sizeof(sockaddr_in_t));
        if (ret < 0)
            goto die;

        ret = recvfrom(fd, rx_buf, sizeof(rx_buf), 0, (sockaddr_t *)&addr, sizeof(sockaddr_in_t));
        if (ret < 0)
            goto die;

        dump(rx_buf, ret);
    }

    return 0;

die:
    perror("ping");
    return 1;
}
