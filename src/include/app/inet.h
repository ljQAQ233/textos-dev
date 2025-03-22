#pragma once

int inet_aton(const char *s, u32 *in);

char *inet_ntoa(u32 in);

u16 htons(u16 h);
u16 ntohs(u16 h);
u32 htonl(u32 h);
u32 ntohl(u32 h);
