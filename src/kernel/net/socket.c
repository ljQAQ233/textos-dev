#include <textos/mm.h>
#include <textos/file.h>
#include <textos/errno.h>
#include <textos/syscall.h>
#include <textos/net/socket.h>

#include "socket/inter.h"

socket_t *socket_get(int fd)
{
    file_t *file = task_current()->files[fd];
    return (socket_t *)file->node->pdata;
}

extern fs_opts_t __socket_opts;

int socket_makefd(socket_t *socket)
{
    int fd;
    file_t *file;
    node_t *sockn;
    if (file_get(&fd, &file, 0) < 0)
        return fd;
    
    sockn = malloc(sizeof(node_t));
    sockn->name = "socket";
    sockn->pdata = socket;
    sockn->opts = &__socket_opts;

    file->node = sockn;
    file->flgs = O_RDWR;
    return fd;
}

/*
 * syscalls
 */
__SYSCALL_DEFINE3(int, socket, int, domain, int, type, int, proto)
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
    
    socket->socktype = socktype;
    socket->op = sockop_get(socktype);
    socket->op->socket(socket);
    socket->nif = nif0;
    return socket_makefd(socket);
}

__SYSCALL_DEFINE3(int, bind, int, fd, sockaddr_t *, addr, socklen_t, len)
{
    socket_t *s = socket_get(fd);
    return s->op->bind(s, addr, len);
}

__SYSCALL_DEFINE2(int, listen, int, fd, int, backlog)
{
    socket_t *s = socket_get(fd);
    return s->op->listen(s, backlog);
}

__SYSCALL_DEFINE3(int, accept, int, fd, sockaddr_t *, addr, socklen_t *, len)
{
    socket_t *s = socket_get(fd);
    return s->op->accept(s, addr, len);
}

__SYSCALL_DEFINE3(int, connect, int, fd, sockaddr_t *, addr, socklen_t, len)
{
    socket_t *s = socket_get(fd);
    return s->op->connect(s, addr, len);
}

__SYSCALL_DEFINE2(int, shutdown, int, fd, int, how)
{
    socket_t *s = socket_get(fd);
    return s->op->shutdown(s, how);
}

__SYSCALL_DEFINE3(int, getsockname, int, fd, sockaddr_t *, addr, socklen_t *, len)
{
    socket_t *s = socket_get(fd);
    return s->op->getsockname(s, addr, len);
}

__SYSCALL_DEFINE3(int, getpeername, int, fd, sockaddr_t *, addr, socklen_t *, len)
{
    socket_t *s = socket_get(fd);
    return s->op->getpeername(s, addr, len);
}

__SYSCALL_DEFINE3(ssize_t, sendmsg, int, fd, msghdr_t *, msg, int, flags)
{
    socket_t *s = socket_get(fd);
    return s->op->sendmsg(s, msg, flags);
}

__SYSCALL_DEFINE3(ssize_t, recvmsg, int, fd, msghdr_t *, msg, int, flags)
{
    socket_t *s = socket_get(fd);
    return s->op->recvmsg(s, msg, flags);
}

__SYSCALL_DEFINE6(ssize_t, sendto,
    int, fd, void *, buf, size_t, len,
    int, flags, sockaddr_t *, dst, socklen_t, dlen)
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

__SYSCALL_DEFINE6(ssize_t, recvfrom,
    int, fd, void *, buf, size_t, len,
    int, flags, sockaddr_t *, src, socklen_t *, slen)
{
    msghdr_t msg = {
        .name = src,
        .namelen = *slen,
        .iov = &(iovec_t) {
            .base = buf,
            .len = len,
        },
        .iovlen = 1,
    };

    int ret = recvmsg(fd, &msg, flags);
    *slen = msg.namelen;
    return ret;
}

extern void sock_raw_init();
extern void sock_udp_init();
extern void sock_tcp_init();

void socket_init()
{
    sock_raw_init();
    sock_udp_init();
    sock_tcp_init();
}
