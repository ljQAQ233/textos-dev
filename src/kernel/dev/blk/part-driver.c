#include <textos/args.h>
#include <textos/dev.h>
#include <textos/dev/blk/part.h>
#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/klib/vsprintf.h>

int part_read(devst_t *dev, blkno_t addr, buffer_t *buf, blkcnt_t cnt, ...)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    struct blk_part_info *bpi = dev->pdata;
    cnt = MIN(cnt, bpi->ptend - addr);
    return prt->bread(prt, addr + bpi->ptoff, buf, cnt);
}

int part_write(devst_t *dev, blkno_t addr, buffer_t *buf, blkcnt_t cnt, ...)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    struct blk_part_info *bpi = dev->pdata;
    cnt = MIN(cnt, bpi->ptend - addr);
    return prt->bwrite(prt, addr + bpi->ptoff, buf, cnt);
}

#include <textos/ioctl.h>

int part_ioctl(devst_t *dev, int req, void *argp)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    switch (req) {
    case BLKSSZGET:
        return prt->ioctl(dev, req, argp);
    default:
        break;
    }
    return -EINVAL;
}
