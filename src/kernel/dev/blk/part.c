#include <textos/dev.h>
#include <textos/dev/blk/mbr.h>
#include <textos/dev/blk/part.h>
#include <textos/dev/buffer.h>
#include <textos/klib/string.h>

extern int part_read(devst_t *, blkno_t, buffer_t *, blkcnt_t, ...);
extern int part_write(devst_t *, blkno_t, buffer_t *, blkcnt_t, ...);
extern int part_ioctl(devst_t *, int, void *);

static int blk_part_fillinfo(struct blk_part *part, struct blk_part_info *info)
{
    info->ptoff = part->relative;
    info->ptend = part->total;
    info->sysid = part->sysid;
    memcpy(&info->record, part, sizeof(info->record));
    return 0;
}

/**
 * @brief scan partitions on a block device `blkdev` and register subdevice
 *
 * @param blkdev block device
 * @return the count of subdevice which re created
 */
int blk_part_scan(devst_t *blkdev)
{
    DEBUGK(K_INFO, "scan %s\n", blkdev->name);

    buffer_t *blk = bread(blkdev, 512, 0);
    if (!blk) {
        DEBUGK(K_ERROR, "cannot get block0 from %s\n", blkdev->name);
        return -1;
    }

    int i = 0;
    struct blk_mbr *mbr = (struct blk_mbr *)blk->blk;
    struct blk_part *ptab = (struct blk_part *)mbr->ptab;
    for (; i < 4; i++) {
        struct blk_part *part = &ptab[i];
        if (!part->sysid) continue;
        struct blk_part_info *info = malloc(sizeof(struct blk_part_info));
        if (!info) break;
        blk_part_fillinfo(part, info);
        blk_part_register(blkdev, info, i);
    }
    DEBUGK(K_INFO, "total %d part(s) found\n", i);
    brelse(blk);
    return i;
}

int blk_part_register(devst_t *blkdev, struct blk_part_info *info, int scan_idx)
{
    scan_idx += 1;
    char name[32];
    blkdev->mkname(blkdev, name, scan_idx);

    devst_t *part = dev_new();
    if (!part) {
        DEBUGK(K_ERROR, "%s cannot get part devst\n", name);
        return -1;
    }

    part->name = strdup(name);
    part->type = DEV_BLK;
    part->subtype = DEV_PART;
    part->bread = part_read;
    part->bwrite = part_write;
    part->ioctl = part_ioctl;
    part->pdata = info;
    dev_register(blkdev, part);
    DEBUGK(K_INFO | K_CONT, "[#%d] %s fs=%x\n", scan_idx, name, info->sysid);
    return 0;
}
