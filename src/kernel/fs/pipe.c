#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/klib/fifo.h>
#include <textos/lock.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>

typedef struct
{
    int alive;
    fifo_t fifo;
    lock_t lock;
    task_t *rd_waiter;
    task_t *wr_waiter;
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

static int pipe_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    size_t rem = siz;
    pipe_t *pi = this->pdata;

    lock_acquire(&pi->lock);
    for (;;) {
        int read = fifo_read(&pi->fifo, buf, rem);
        if (read == 0) {
            if (pi->alive == 1) break;
            block_as(&pi->lock, &pi->rd_waiter);
            continue;
        }
        rem -= read;
        buf += read;
        if (pi->wr_waiter) task_unblock(pi->wr_waiter, 0);
        if (!rem) break;
    }
    lock_release(&pi->lock);

    return siz - rem;
}

static int pipe_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    size_t reqsiz = siz;
    pipe_t *pi = this->pdata;
    if (pi->alive == 1) return 0;

    lock_acquire(&pi->lock);
    for (;;) {
        int write = fifo_write(&pi->fifo, buf, siz);
        siz -= write;
        buf += write;
        if (pi->rd_waiter) task_unblock(pi->rd_waiter, 0);
        if (!siz) break;
        if (pi->alive == 1) break;
        block_as(&pi->lock, &pi->wr_waiter);
    }
    lock_release(&pi->lock);

    return reqsiz - siz;
}

static int pipe_close(node_t *this)
{
    pipe_t *pi = this->pdata;
    pi->alive--;
    if (!pi->alive) {
        if (pi->rd_waiter) task_unblock(pi->rd_waiter, 0);
        if (pi->wr_waiter) task_unblock(pi->wr_waiter, 0);
        free(pi);
    }
    return 0;
}

static fs_opts_t __pipe_rops = {};
static fs_opts_t __pipe_wops = {};

int pipe_init(node_t *pipe0, node_t *pipe1)
{
    void *pb = vmm_allocpages(1, PE_P | PE_RW);
    size_t ps = PAGE_SIZ;
    pipe_t *pi = malloc(sizeof(pipe_t));
    pi->rd_waiter = NULL;
    pi->wr_waiter = NULL;
    pi->alive = 2;
    fifo_init(&pi->fifo, pb, ps);
    lock_init(&pi->lock);

    pipe0->name = "pipe";
    pipe0->opts = &__pipe_rops;
    pipe0->pdata = pi;

    pipe1->name = "pipe";
    pipe1->opts = &__pipe_wops;
    pipe1->pdata = pi;

    return 0;
}

static int bad_peer()
{
    return -EBADF;
}

// vfs pipe initializer
void __pipe_init()
{
    vfs_initops(&__pipe_rops);
    __pipe_rops.read = pipe_read;
    __pipe_rops.write = (void *)bad_peer;
    __pipe_rops.close = pipe_close;

    vfs_initops(&__pipe_wops);
    __pipe_wops.read = (void *)bad_peer;
    __pipe_wops.write = pipe_write;
    __pipe_wops.close = pipe_close;
}
