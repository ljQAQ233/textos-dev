#ifndef _ARPA_INET_H
#define	_ARPA_INET_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_uint16_t
#define __NEED_uint32_t
#include <bits/alltypes.h>

int inet_aton(const char *s, uint32_t *in);

char *inet_ntoa(uint32_t in);

uint16_t htons(uint16_t h);
uint16_t ntohs(uint16_t h);
uint32_t htonl(uint32_t h);
uint32_t ntohl(uint32_t h);

__END_DECLS

#endif