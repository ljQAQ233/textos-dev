// inet converter
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int inet_aton(const char *s, struct in_addr *in)
{
    uint8_t *a = (uint8_t *)in;
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

char *inet_ntoa(uint32_t in)
{
	static char buf[16];
	uint8_t *a = (uint8_t *)&in;
	sprintf(buf, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
	return buf;
}

static inline uint16_t bswap16(uint16_t x)
{
	return x << 8 | x >> 8;
}

static inline uint32_t bswap32(uint32_t x)
{
	return x >> 24 | x >> 8 & 0xff00 | x << 8 & 0xff0000 | x << 24;
}

uint16_t htons(uint16_t h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap16(h) : h;
}

uint16_t ntohs(uint16_t h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap16(h) : h;
}

uint32_t htonl(uint32_t h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap32(h) : h;
}

uint32_t ntohl(uint32_t h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap32(h) : h;
}
