/*
 * network uses big-endian byte order
 */

#ifndef CONFIG_BYTE_BE

u16 htons(u16 h)
{
    return ((h & 0xFF00) >> 8) | ((h & 0x00FF) << 8);
}

u16 ntohs(u16 h)
{
    return ((h & 0xFF00) >> 8) | ((h & 0x00FF) << 8);
}

u32 htonl(u32 h)
{
    return ((h & 0xFF000000) >> 24) |
           ((h & 0x00FF0000) >> 8)  |
           ((h & 0x0000FF00) << 8)  |
           ((h & 0x000000FF) << 24);
}

u32 ntohl(u32 h)
{
    return ((h & 0xFF000000) >> 24) |
           ((h & 0x00FF0000) >> 8)  |
           ((h & 0x0000FF00) << 8)  |
           ((h & 0x000000FF) << 24);
}

#else

u16 htons(u16 h)
{
    return h;
}

u16 ntohs(u16 h)
{
    return h;
}

u32 htonl(u32 h)
{
    return h;
}

u32 ntohl(u32 h)
{
    return h;
}

#endif

#include <textos/net.h>

nic_t *nic0;

