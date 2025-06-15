#ifndef	_NETINET_IN_H
#define	_NETINET_IN_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_uint8_t
#define __NEED_uint16_t
#define __NEED_uint32_t
#define __NEED_sa_family_t
#include <bits/alltypes.h>

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
struct in_addr { in_addr_t s_addr; };

struct sockaddr_in
{
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;
    uint8_t sin_zero[8];
};

uint16_t htons(uint16_t h);
uint16_t ntohs(uint16_t h);
uint32_t htonl(uint32_t h);
uint32_t ntohl(uint32_t h);

/* Standard well-defined IP protocols.  */
#define IPPROTO_IP   0
#define IPPROTO_ICMP 1
#define IPPROTO_TCP  6
#define IPPROTO_UDP  17
#define IPPROTO_RAW  255

__END_DECLS

#endif