#ifndef _NET_IF_H
#define _NET_IF_H

#include <sys/cdefs.h>

__BEGIN_DECLS

/* os-specified */

#include <sys/socket.h>

#define IFNAMSIZ 16

struct ifreq
{
    char ifr_name[IFNAMSIZ];
    
    union
    {
        __uint8_t ifr_addr[4];
        __uint8_t ifr_dstaddr[4];
        __uint8_t ifr_broadaddr[4];
        __uint8_t ifr_netmask[4];
        __uint8_t ifr_hwaddr[4];
    };
};

__END_DECLS

#endif