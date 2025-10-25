#ifndef _ARPA_INET_H
#define	_ARPA_INET_H

#include <sys/cdefs.h>

__BEGIN_DECLS

struct in_addr;

#define __NEED_uint16_t
#define __NEED_uint32_t
#include <bits/alltypes.h>

int inet_aton(const char *__s, struct in_addr *__in);

char *inet_ntoa(uint32_t __in);

uint16_t htons(uint16_t __h);
uint16_t ntohs(uint16_t __h);
uint32_t htonl(uint32_t __h);
uint32_t ntohl(uint32_t __h);

__END_DECLS

#endif
