/*
 * dns query - written by deepseek
 */

#include <app/api.h>
#include <app/inet.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    u16 id;
    u16 flags;
    u16 qdcount;
    u16 ancount;
    u16 nscount;
    u16 arcount;
} hdr_t;

typedef struct
{
    u16 qtype;
    u16 qclass;
} quest_t;

void mkqry(const char *domain, u8 *buf, size_t *len)
{
    hdr_t *hdr = (hdr_t *)buf;
    hdr->id = htons(0x1234);
    hdr->flags = htons(0x0100);
    hdr->qdcount = htons(1);
    hdr->ancount = 0;
    hdr->nscount = 0;
    hdr->arcount = 0;

    u8 *ptr = buf + sizeof(hdr_t);
    const char *token = domain;
    while (*token)
    {
        const char *dot = strchr(token, '.');
        size_t len = dot ? (dot - token) : strlen(token);
        *ptr++ = len;
        memcpy(ptr, token, len);
        ptr += len;
        token = dot ? dot + 1 : token + len;
    }
    *ptr++ = 0;

    quest_t *quest = (quest_t *)ptr;
    quest->qtype = htons(1);  // Type A record
    quest->qclass = htons(1); // Class IN

    *len = (ptr - buf) + sizeof(quest_t);
}

void parse(u8 *rx, ssize_t rb)
{
    hdr_t *hdr = (hdr_t *)rx;
    printf("ID: %04x\n", ntohs(hdr->id));
    printf("Flags: %04x\n", ntohs(hdr->flags));
    printf("Questions: %d\n", ntohs(hdr->qdcount));
    printf("Answers: %d\n", ntohs(hdr->ancount));

    u8 *ptr = rx + sizeof(hdr_t);
    while (*ptr != 0)
        ptr++;
    ptr += 5;

    for (int i = 0; i < ntohs(hdr->ancount); i++)
    {
        if ((*ptr & 0xC0) == 0xC0)
        {
            ptr += 2;
        }
        else
        {
            while (*ptr != 0)
                ptr++;
            ptr++;
        }

        u16 type = ntohs(*(u16 *)ptr);
        ptr += 2;
        u16 class = ntohs(*(u16 *)ptr);
        ptr += 2;
        u32 ttl = ntohl(*(u32 *)ptr);
        ptr += 4;
        u16 rdlength = ntohs(*(u16 *)ptr);
        ptr += 2;

        if (type == 1 && class == 1)
        {
            u32 ip = ntohl(*(u32 *)ptr);
            printf("IP: %d.%d.%d.%d\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
        }

        ptr += rdlength;
    }
}

u8 tx[512];
u8 rx[512];

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("usage: %s <domain>\n", argv[0]);
        return 1;
    }

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in_t addr;
    memset(&addr, 0, sizeof(addr));
    addr.family = AF_INET;
    addr.port = htons(53);
    inet_aton("180.76.76.76", &addr.addr);

    size_t len;
    mkqry(argv[1], tx, &len);

    ssize_t sent = sendto(fd, tx, len, 0, (void *)&addr, sizeof(addr));
    if (sent < 0)
    {
        perror("sendto");
        close(fd);
        return 1;
    }

    sockaddr_in_t from;
    ssize_t rb = recvfrom(fd, rx, sizeof(rx), 0, (void *)&from, sizeof(from));
    if (rb < 0)
    {
        perror("recvfrom");
        close(fd);
        return 1;
    }

    parse(rx, rb);

    return 0;
}
