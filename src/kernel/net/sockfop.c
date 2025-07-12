#include <textos/net.h>
#include <textos/net/socket.h>
#include <textos/fs.h>
#include <textos/file.h>

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

static int socket_close(node_t *this)
{
    socket_t *s = this->pdata;
    return s->op->shutdown(s, SHUT_RDWR);
}

// XXX: recvmsg need a pointer to address_len
static int socket_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    socket_t *s = this->pdata;
    msghdr_t msg = {
        .name = NULL,
        .namelen = 0,
        .iov = &(iovec_t) {
            .iov_base = buf,
            .iov_len = siz,
        },
        .iovlen = 1,
    };
    return s->op->recvmsg(s, &msg, 0);
}

static int socket_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    msghdr_t msg = {
        .name = NULL,
        .namelen = 0,
        .iov = &(iovec_t) {
            .iov_base = buf,
            .iov_len = siz,
        },
        .iovlen = 1,
    };
    socket_t *s = this->pdata;
    return s->op->sendmsg(s, &msg, 0);
}

fs_opts_t __socket_opts = {
    noopt,
    noopt,
    noopt,
    noopt,
    noopt,
    noopt,
    noopt,
    socket_read,
    socket_write,
    noopt,
    noopt,
    socket_ioctl,
    socket_close,
};