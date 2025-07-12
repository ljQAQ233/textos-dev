#pragma once

#include <textos/net.h>
#include <textos/lock.h>

/* address family */
#define AF_UNSPEC 0
#define AF_INET   2
#define AF_PACKET 17
#define AF_MAX    45

#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3
#define SOCK_PACKET 10

typedef struct sockaddr_t
{
    u16 family;
    char data[14];
} sockaddr_t;

typedef struct sockaddr_in_t
{
    u16 family;
    u16 port;
    ipv4_t addr;
    u8 zero[8];
} sockaddr_in_t;

struct socket;
typedef struct socket socket_t;

enum
{
    SOCK_T_NONE,
    SOCK_T_RAW,
    SOCK_T_PKT,
    SOCK_T_UDP,
    SOCK_T_TCP,
    SOCK_T_MAX,
};

typedef struct
{
    sockaddr_t *name;
    int namelen;
    iovec_t *iov;
    size_t iovlen;
} msghdr_t;

typedef struct
{
    int (*socket)(socket_t *s);
    int (*bind)(socket_t *s, sockaddr_t *addr, socklen_t len);
    int (*listen)(socket_t *s, int backlog);
    int (*accept)(socket_t *s, sockaddr_t *addr, socklen_t *len);
    int (*connect)(socket_t *s, sockaddr_t *addr, socklen_t len);
    int (*shutdown)(socket_t *s, int how);
    int (*getsockname)(socket_t *s, sockaddr_t *addr, socklen_t *len);
    int (*getpeername)(socket_t *s, sockaddr_t *addr, socklen_t *len);
    ssize_t (*sendmsg)(socket_t *s, msghdr_t *msg, int flags);
    ssize_t (*recvmsg)(socket_t *s, msghdr_t *msg, int flags);
} sockop_t;

struct socket
{
    // init arg
    int domain;
    int type;
    int proto;
    void *pri;

    int shutdown;

    // special
    int socktype;
    sockop_t *op;
    nif_t *nif;
    list_t intype;
};

socket_t *socket_get(int fd);

int socket(int domain, int type, int proto);

int bind(int fd, sockaddr_t *addr, socklen_t len);
int connect(int fd, sockaddr_t *addr, socklen_t len);
int listen(int fd, int backlog);
int accept(int fd, sockaddr_t *addr, socklen_t *len);

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2

int shutdown(int fd, int how);

int getsockname(int fd, sockaddr_t *addr, socklen_t *len);
int getpeername(int fd, sockaddr_t *addr, socklen_t *len);

ssize_t sendmsg(int fd, msghdr_t *msg, int flags);
ssize_t recvmsg(int fd, msghdr_t *msg, int flags);

ssize_t sendto(int fd, void *buf, size_t len, int flags, sockaddr_t *dst, socklen_t dlen);
ssize_t recvfrom(int fd, void *buf, size_t len, int flags, sockaddr_t *src, socklen_t *slen);

#define IFNAMSIZ 16

typedef struct ifreq
{
    char name[IFNAMSIZ];
    
    union
    {
        ipv4_t addr;
        ipv4_t dstaddr;
        ipv4_t broadaddr;
        ipv4_t netmask;
        mac_t  hwaddr;
    };
} ifreq_t;
