#include <textos/fs.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>
#include <textos/lock.h>
#include <textos/klib/fifo.h>

#include <textos/assert.h>

typedef struct
{
    fifo_t fifo;
    lock_t lock;
    int rd_waiter;
    int wr_waiter;
} pipe_t;

void block_as(lock_t *lock, int *as)
{
    // only one task is supported
    ASSERTK(*as == -1);

    *as = task_current()->pid;
    lock_release(lock);
    task_block();
    lock_acquire(lock);
    *as = -1;
}

static int pipe_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    pipe_t *pi = this->pdata;
    lock_acquire(&pi->lock);
    for (;;) 
    {
        int read = fifo_read(&pi->fifo, buf, siz);
        siz -= read;
        buf += read;
        if (pi->wr_waiter >= 0)
            task_unblock(pi->wr_waiter);
        if (!siz)
            break;
        block_as(&pi->lock, &pi->rd_waiter);
    }
    lock_release(&pi->lock);
}

static int pipe_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    pipe_t *pi = this->pdata;
    lock_acquire(&pi->lock);
    for (;;)
    {
        int write = fifo_write(&pi->fifo, buf, siz);
        siz -= write;
        buf += write;
        if (pi->rd_waiter >= 0)
            task_unblock(pi->rd_waiter);
        if (!siz)
            break;
        block_as(&pi->lock, &pi->wr_waiter);
    }
    lock_release(&pi->lock);
}

static int pipe_close(node_t *this)
{
    // todo : wake up & handle
    return 0;
}

fs_opts_t __pipe_opts = { };

int pipe_init(node_t *pipe)
{
    void *pb = vmm_allocpages(1, PE_P | PE_RW);
    size_t ps = PAGE_SIZ;
    pipe_t *pi = malloc(sizeof(pipe_t));
    pi->rd_waiter = -1;
    pi->wr_waiter = -1;
    fifo_init(&pi->fifo, pb, ps);
    lock_init(&pi->lock);
    
    pipe->name = "pipe";
    pipe->pdata = pi;
    pipe->opts = &__pipe_opts;

    return 0;
}

// vfs pipe initializer
int __pipe_init()
{
    fs_opts_t *opts = &__pipe_opts;

    vfs_initops(opts);
    opts->read = pipe_read;
    opts->write = pipe_write;
    opts->close = pipe_close;
}