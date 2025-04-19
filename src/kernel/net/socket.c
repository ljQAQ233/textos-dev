#include <textos/mm.h>
#include <textos/file.h>
#include <textos/errno.h>
#include <textos/net/socket.h>

#include "socket/inter.h"

socket_t *socket_get(int fd)
{
    file_t *file = task_current()->files[fd];
    return (socket_t *)file->node->pdata;
}

static fs_opts_t __socket_opts;

int socket_makefd(socket_t *socket)
{
    int fd;
    file_t *file;
    node_t *sockn;
    if (file_get(&fd, &file) < 0)
        return fd;
    
    sockn = malloc(sizeof(node_t));
    sockn->name = "socket";
    sockn->pdata = socket;
    sockn->opts = &__socket_opts;

    file->node = sockn;
    return fd;
}

int socket(int domain, int type, int proto)
{
    socket_t *socket;

    int socktype = SOCK_T_NONE;
    switch (domain)
    {
    case AF_INET:
        switch (type)
        {
        case SOCK_RAW:
            socktype = SOCK_T_RAW;
            break;

        case SOCK_DGRAM:
            socktype = SOCK_T_UDP;
            break;

        case SOCK_STREAM:
            socktype = SOCK_T_TCP;
            break;

        case SOCK_PACKET:
            socktype = SOCK_T_PKT;
            break;

        default:
            break;
        }
        break;

    case AF_PACKET:
        socktype = SOCK_T_PKT;
        break;

    default:
        return -EINVAL;
    }

    socket = malloc(sizeof(socket_t));
    socket->domain = domain;
    socket->type = type;
    socket->proto = proto;

    socket->rx_waiter = -1;
    list_init(&socket->rx_queue);
    lock_init(&socket->lock);
    
    socket->socktype = socktype;
    socket->op = sockop_get(socktype);
    socket->op->socket(socket);
    socket->nif = nif0;
    return socket_makefd(socket);
}

int bind(int fd, sockaddr_t *addr, size_t len)
{
    socket_t *s = socket_get(fd);
    return s->op->bind(s, addr, len);
}

int listen(int fd, int backlog)
{
    socket_t *s = socket_get(fd);
    return s->op->listen(s, backlog);
}

int accept(int fd, sockaddr_t *addr, size_t *len)
{
    socket_t *s = socket_get(fd);
    return s->op->accept(s, addr, len);
}

int connect(int fd, sockaddr_t *addr, size_t len)
{
    socket_t *s = socket_get(fd);
    return s->op->connect(s, addr, len);
}

int getsockname(int fd, sockaddr_t *addr, size_t len)
{
    socket_t *s = socket_get(fd);
    return s->op->getsockname(s, addr, len);
}

int getpeername(int fd, sockaddr_t *addr, size_t len)
{
    socket_t *s = socket_get(fd);
    return s->op->getpeername(s, addr, len);
}

ssize_t sendmsg(int fd, msghdr_t *msg, int flags)
{
    socket_t *s = socket_get(fd);
    return s->op->sendmsg(s, msg, flags);
}

ssize_t recvmsg(int fd, msghdr_t *msg, int flags)
{
    socket_t *s = socket_get(fd);
    return s->op->recvmsg(s, msg, flags);
}

ssize_t sendto(int fd, void *buf, size_t len, int flags, sockaddr_t *dst, size_t dlen)
{
    msghdr_t msg = {
        .name = dst,
        .namelen = dlen,
        .iov = &(iovec_t) {
            .base = buf,
            .len = len,
        },
        .iovlen = 1,
    };

    return sendmsg(fd, &msg, flags);
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, sockaddr_t *src, size_t slen)
{
    msghdr_t msg = {
        .name = src,
        .namelen = slen,
        .iov = &(iovec_t) {
            .base = buf,
            .len = len,
        },
        .iovlen = 1,
    };

    return recvmsg(fd, &msg, flags);
}

#include <string.h>

static int socket_ioctl(node_t *this, int req, void *argp)
{
    ifreq_t *ifr = argp;
    socket_t *socket = this->pdata;

    //
    // TODO: handle its own req first here
    //

    nif_t *nif = nif_find(ifr->name);
    return nif_ioctl(nif, req, argp);
}

extern void sock_raw_init();
extern void sock_udp_init();
extern void sock_tcp_init();

void socket_init()
{
    vfs_initops(&__socket_opts);
    __socket_opts.ioctl = socket_ioctl;

    sock_raw_init();
    sock_udp_init();
    sock_tcp_init();
}
