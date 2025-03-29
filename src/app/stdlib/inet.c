// inet converter

#include <app/api.h>
#include <stdio.h>

int inet_aton(const char *s, u32 *in)
{
    u8 *a = (u8 *)in;
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

char *inet_ntoa(u32 in)
{
	static char buf[16];
	u8 *a = (u8 *)&in;
	sprintf(buf, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
	return buf;
}

static inline u16 bswap16(u16 x)
{
	return x << 8 | x >> 8;
}

static inline u32 bswap32(u32 x)
{
	return x >> 24 | x >> 8 & 0xff00 | x << 8 & 0xff0000 | x << 24;
}

u16 htons(u16 h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap16(h) : h;
}

u16 ntohs(u16 h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap16(h) : h;
}

u32 htonl(u32 h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap32(h) : h;
}

u32 ntohl(u32 h)
{
	union { int i; char c; } u = { 1 };
	return u.c ? bswap32(h) : h;
}