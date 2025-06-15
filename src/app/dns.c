/*
 * dns query - written by deepseek
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct
{
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} hdr_t;

typedef struct
{
    uint16_t qtype;
    uint16_t qclass;
} quest_t;

void mkqry(const char *domain, uint8_t *buf, size_t *len)
{
    hdr_t *hdr = (hdr_t *)buf;
    hdr->id = htons(0x1234);
    hdr->flags = htons(0x0100);
    hdr->qdcount = htons(1);
    hdr->ancount = 0;
    hdr->nscount = 0;
    hdr->arcount = 0;

    uint8_t *ptr = buf + sizeof(hdr_t);
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

void parse(uint8_t *rx, ssize_t rb)
{
    hdr_t *hdr = (hdr_t *)rx;
    printf("ID: %04x\n", ntohs(hdr->id));
    printf("Flags: %04x\n", ntohs(hdr->flags));
    printf("Questions: %d\n", ntohs(hdr->qdcount));
    printf("Answers: %d\n", ntohs(hdr->ancount));

    uint8_t *ptr = rx + sizeof(hdr_t);
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

        uint16_t type = ntohs(*(uint16_t *)ptr);
        ptr += 2;
        uint16_t class = ntohs(*(uint16_t *)ptr);
        ptr += 2;
        uint32_t ttl = ntohl(*(uint32_t *)ptr);
        ptr += 4;
        uint16_t rdlength = ntohs(*(uint16_t *)ptr);
        ptr += 2;

        if (type == 1 && class == 1)
        {
            uint32_t ip = ntohl(*(uint32_t *)ptr);
            printf("IP: %d.%d.%d.%d\n", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
        }

        ptr += rdlength;
    }
}

uint8_t tx[512];
uint8_t rx[512];

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

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    inet_aton("180.76.76.76", &addr.sin_addr.s_addr);

    size_t len;
    mkqry(argv[1], tx, &len);

    ssize_t sent = sendto(fd, tx, len, 0, (void *)&addr, sizeof(addr));
    if (sent < 0)
    {
        perror("sendto");
        close(fd);
        return 1;
    }

    struct sockaddr_in from;
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
