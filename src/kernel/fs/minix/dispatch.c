#include <textos/dev.h>
#include <textos/dev/buffer.h>

#include "minix.h"

superblk_t *__fs_init_minix(devst_t *dev)
{
    buffer_t *blk;
    u16 *pmagic;
    superblk_t *sb;
    blk = bread(dev, 1024, 1);
    if (blk == NULL) {
        DEBUGK(K_ERROR, "cannot read minix superblock\n");
        return NULL;
    }

    pmagic = (u16 *)(blk->blk + V1_SUPER_MAGIC_OFFSET);
    if (*pmagic == MINIX_V1) {
        sb = __fs_init_minix1(dev, blk);
    } else if (*pmagic == MINIX_V2) {
        sb = NULL;
    } else {
        pmagic = (u16 *)(blk->blk + V3_SUPER_MAGIC_OFFSET);
        if (*pmagic == MINIX_V3)
            sb = __fs_init_minix3(dev, blk);
        else
            sb = NULL;
    }
    brelse(blk);
    return sb;
}
