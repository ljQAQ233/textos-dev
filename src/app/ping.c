#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

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
    struct sockaddr_in sip;
    size_t slen = sizeof(sip);

    struct ip *ip = (struct ip *)buf;
    struct icmp *hdr = (struct icmp *)(buf + ip->ip_hl * 4);

    if (hdr->icmp_type == ICMP_ECHOREPLY)
    {
        printf("%d bytes from %s: icmp_seq=%d ttl=%d\n",
            len, inet_ntoa(ip->ip_src.s_addr),
            hdr->icmp_seq, ip->ip_ttl);
    }
}

extern int inet_aton(const char *s, uint32_t *in);

int main(int argc, char const *argv[])
{
    int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd < 0)
        goto die;

    struct sockaddr_in addr;
    if (inet_aton(argv[1], &addr.sin_addr.s_addr) == 0)
        goto die;
    
    uint16_t seq = 1;
    char tx_buf[128];
    char rx_buf[128];

    struct icmp *hdr = (struct icmp *)tx_buf;
    hdr->icmp_type = ICMP_ECHO;
    hdr->icmp_code = 0;
    hdr->icmp_id = htons(1);
    hdr->icmp_seq = htons(seq);
    hdr->icmp_cksum = 0;
    hdr->icmp_cksum = cksum(hdr, sizeof(struct icmp));

    // 8
    int len = sizeof(*hdr);
    for (;;)
    {
        int ret = sendto(fd, tx_buf, len, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
        if (ret < 0)
            goto die;

        ret = recvfrom(fd, rx_buf, sizeof(rx_buf), 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
        if (ret < 0)
            goto die;

        dump(rx_buf, ret);
    }

    return 0;

die:
    perror("ping");
    return 1;
}
