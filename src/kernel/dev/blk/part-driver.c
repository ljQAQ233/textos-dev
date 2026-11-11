#include <textos/args.h>
#include <textos/dev.h>
#include <textos/dev/blk/part.h>
#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/klib/vsprintf.h>

static int blk_part_read(devst_t *dev, blkno_t addr, buffer_t *buf,
                         blkcnt_t cnt, ...)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    struct blk_part_info *bpi = dev->pdata;
    cnt = MIN(cnt, bpi->ptend - addr);
    return prt->op->bread(prt, addr + bpi->ptoff, buf, cnt);
}

static int blk_part_write(devst_t *dev, blkno_t addr, buffer_t *buf,
                          blkcnt_t cnt, ...)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    struct blk_part_info *bpi = dev->pdata;
    cnt = MIN(cnt, bpi->ptend - addr);
    return prt->op->bwrite(prt, addr + bpi->ptoff, buf, cnt);
}

#include <textos/ioctl.h>

static int blk_part_ioctl(devst_t *dev, int req, void *argp, ...)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    switch (req) {
    case BLKSSZGET:
        return prt->op->ioctl(dev, req, argp);
    default:
        break;
    }
    return -EINVAL;
}

static dev_opts_t blk_part_op = {
    .bread = blk_part_read,
    .bwrite = blk_part_write,
    .ioctl = blk_part_ioctl,
};

void blk_part_getops(dev_opts_t **pp)
{
    *pp = &blk_part_op;
}
