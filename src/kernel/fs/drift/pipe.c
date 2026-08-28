#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/fs/poll.h>
#include <textos/klib/fifo.h>
#include <textos/lock.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>

static fs_opts_t __pipe_rops = {};
static fs_opts_t __pipe_wops = {};

#define is_rpipe(n) ((n)->opts == &__pipe_rops)
#define is_wpipe(n) ((n)->opts == &__pipe_wops)

typedef struct
{
    int alive;
    fifo_t fifo;
    lock_t lock;
    task_t *rd_waiter;
    task_t *wr_waiter;
    struct fs_pollee rd_pollee;
    struct fs_pollee wr_pollee;
} pipe_t;

static void block_as(lock_t *lock, task_t **as)
{
    // only one task is supported
    ASSERTK(*as == NULL);
    *as = task_current();
    lock_release(lock);
    task_block(NULL, NULL, TASK_BLK, 0);
    lock_acquire(lock);
    *as = NULL;
}

static int pipe_read(node_t *this, void *buf, size_t siz, size_t offset,
                     struct fs_openctx *openctx)
{
    size_t rem = siz;
    pipe_t *pi = this->pdata;
    if (rem == 0) return 0;

    lock_acquire(&pi->lock);
    for (;;) {
        int read = fifo_read(&pi->fifo, buf, rem);
        if (read == 0) {
            if (pi->alive == 1) break;
            if (openctx->file_flgs & O_NONBLOCK) break;
            block_as(&pi->lock, &pi->rd_waiter);
            continue;
        }
        rem -= read;
        buf += read;
        fs_poll_deliver(&pi->wr_pollee, POLLOUT);
        if (pi->wr_waiter) task_unblock(pi->wr_waiter, 0);
        break;
    }
    lock_release(&pi->lock);

    return siz - rem;
}

static int pipe_write(node_t *this, void *buf, size_t siz, size_t offset,
                      struct fs_openctx *openctx)
{
    size_t reqsiz = siz;
    pipe_t *pi = this->pdata;
    if (reqsiz == 0) return 0;
    if (pi->alive == 1) return 0;

    // TODO: use more accurate variable names
    lock_acquire(&pi->lock);
    for (;;) {
        int write = fifo_write(&pi->fifo, buf, siz);
        if (write == 0) {
            block_as(&pi->lock, &pi->wr_waiter);
            continue;
        }
        siz -= write;
        buf += write;
        fs_poll_deliver(&pi->rd_pollee, POLLIN);
        if (pi->rd_waiter) task_unblock(pi->rd_waiter, 0);
        if (!siz) break;
        if (pi->alive == 1) break;
        if (openctx->file_flgs & O_NONBLOCK) break;
    }
    lock_release(&pi->lock);

    return reqsiz - siz;
}

static int pipe_close(node_t *this)
{
    pipe_t *pi = this->pdata;
    pi->alive--;
    if (pi->alive == 1) {
        fs_poll_deliver(&pi->wr_pollee, POLLHUP);
        fs_poll_deliver(&pi->rd_pollee, POLLHUP);
        if (pi->rd_waiter) task_unblock(pi->rd_waiter, 0);
        if (pi->wr_waiter) task_unblock(pi->wr_waiter, 0);
    } else if (pi->alive == 0) {
        free(pi);
    }
    return 0;
}

static int pipe_poll_setup(node_t *this, struct fs_poller *poller)
{
    pipe_t *pi = this->pdata;
    if (is_rpipe(this)) {
        if (pi->alive == 1) {
            if (fs_poll_deliver_to(&pi->rd_pollee, POLLHUP, poller, false))
                return 1;
        }
        if (!fifo_empty(&pi->fifo)) {
            if (fs_poll_deliver_to(&pi->rd_pollee, POLLIN, poller, false))
                return 1;
        }
        fs_poll_setup(&pi->rd_pollee, poller);
    } else if (is_wpipe(this)) {
        if (pi->alive == 1) {
            if (fs_poll_deliver_to(&pi->wr_pollee, POLLHUP, poller, false))
                return 1;
        }
        if (!fifo_full(&pi->fifo)) {
            if (fs_poll_deliver_to(&pi->wr_pollee, POLLOUT, poller, false))
                return 1;
        }
        fs_poll_setup(&pi->wr_pollee, poller);
    } else {
        return -1;
    }
    return 0;
}

static int pipe_poll_teardown(node_t *this, struct fs_poller *poller)
{
    pipe_t *pi = this->pdata;
    if (is_rpipe(this)) {
        fs_poll_teardown(&pi->rd_pollee, poller);
    } else if (is_wpipe(this)) {
        fs_poll_teardown(&pi->wr_pollee, poller);
    } else {
        return -1;
    }
    return 0;
}

int vfs_pipe_create(node_t *pipe0, node_t *pipe1)
{
    void *pb = vmm_allocpages(1, PE_P | PE_RW);
    size_t ps = PAGE_SIZ;
    pipe_t *pi = malloc(sizeof(pipe_t));
    pi->alive = 2;
    fifo_init(&pi->fifo, pb, ps);
    lock_init(&pi->lock);
    pi->rd_waiter = NULL;
    pi->wr_waiter = NULL;
    fs_pollee_init(&pi->rd_pollee);
    fs_pollee_init(&pi->wr_pollee);

    pipe0->name = "rpipe";
    pipe0->opts = &__pipe_rops;
    pipe0->pdata = pi;

    pipe1->name = "wpipe";
    pipe1->opts = &__pipe_wops;
    pipe1->pdata = pi;

    return 0;
}

static int bad_peer()
{
    return -EBADF;
}

// vfs pipe initializer
void __vfs_pipe_init()
{
    vfs_initops(&__pipe_rops);
    __pipe_rops.read = pipe_read;
    __pipe_rops.write = (void *)bad_peer;
    __pipe_rops.close = pipe_close;
    __pipe_rops.poll_setup = pipe_poll_setup;
    __pipe_rops.poll_teardown = pipe_poll_teardown;

    vfs_initops(&__pipe_wops);
    __pipe_wops.read = (void *)bad_peer;
    __pipe_wops.write = pipe_write;
    __pipe_wops.close = pipe_close;
    __pipe_rops.poll_setup = pipe_poll_setup;
    __pipe_rops.poll_teardown = pipe_poll_teardown;
}
