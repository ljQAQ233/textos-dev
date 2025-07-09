#include <textos/fs.h>
#include <textos/mm.h>
#include <textos/dev.h>
#include <textos/errno.h>
#include <textos/fs/inter.h>
#include <textos/dev/buffer.h>
#include <textos/klib/stack.h>
#include <textos/klib/string.h>

static inline devst_t *dev_extract(node_t *node)
{
    uint ma = major(node->rdev);
    uint mi = minor(node->rdev);
    return dev_lookup_nr(ma, mi);
}

static int dev_ioctl(node_t *this, int req, void *argp)
{
    devst_t *dev = dev_extract(this);
    if (!dev)
        return -EINVAL;
    return dev->ioctl(dev, req, argp);
}

static int dev_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    devst_t *dev = dev_extract(this);
    if (!dev)
        return -EINVAL;
    if (dev->type == DEV_CHAR)
        return dev->read(dev, buf, siz);

    int blksiz;
    dev->ioctl(dev, BLKSSZGET, &blksiz);
    int blk = offset / blksiz;
    int off = offset % blksiz;

    // 在 fat32 中写过这样的代码
    // off 是 src 在块中的偏移, 只有第一个读取的块可能 off != 0
    // 每一次最大读取的字节数就是块的大小
    // 对于最后一个块, 可能不会完全读取, 所以取最小 siz
    buffer_t *b;
    for (int rem = siz ; rem ; blk++) {
        b = bread(dev, blksiz, blk);

        int cpy = MIN(off != 0 ? blksiz - off : blksiz, rem);
        memcpy(buf, b->blk + off, cpy);
        rem -= cpy;
        off = 0;

        brelse(b);
    }
    return siz;
}

static int dev_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    devst_t *dev = dev_extract(this);
    if (!dev)
        return -EINVAL;
    if (dev->type == DEV_CHAR)
        return dev->write(dev, buf, siz);

    int blksiz;
    dev->ioctl(dev, BLKSSZGET, &blksiz);
    int blk = offset / blksiz;
    int off = offset % blksiz;

    buffer_t *b;
    for (int rem = siz ; rem ; blk++) {
        b = bread(dev, blksiz, blk);

        int cpy = MIN(off != 0 ? blksiz - off : blksiz, rem);
        memcpy(b->blk + off, buf, cpy);
        rem -= cpy;
        off = 0;

        bdirty(b, true);
        brelse(b);
    }
    return siz;
}

static int dev_close(node_t *this)
{
    return 0;
}

static void *dev_mmap(node_t *this, vm_region_t *vm)
{
    devst_t *dev = dev_extract(this);
    if (!dev)
        return MRET(-EINVAL);
    return dev->mmap(dev, vm);
}

fs_opts_t __vfs_dev_op = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    dev_read,
    dev_write,
    NULL,
    dev_mmap,
    dev_ioctl,
    dev_close,
};