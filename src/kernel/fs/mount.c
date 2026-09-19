#include <textos/dev.h>
#include <textos/dev/blk/part.h>
#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/fs/internal-registry.h>

extern superblk_t *__fs_init_fat32(devst_t *dev);
extern superblk_t *__fs_init_minix(devst_t *dev);

// clang-format off

static struct fs_registry fsregs[] = {
    [FS_FAT32] = {
        .name = "fat32",
        .id = 0xc,
        .init = __fs_init_fat32
    },
    [FS_MINIX1] = {
        .name = "minix1",
        .id = 0x81,
        .init = __fs_init_minix
    },
    {
        .name = "endsym",
        .id = 0,
        .init = NULL
    }
};

// clang-format on

int fs_extract_mount(devst_t *blkdev, node_t **root)
{
    if (blkdev->type != DEV_BLK) return -EINVAL;
    if (blkdev->subtype != DEV_PART) return -EINVAL;
    struct blk_part_info *bpi = blkdev->pdata;
    struct fs_registry *look = fsregs;
    struct superblk *sb;
    for (; look->init; look++) {
        if (look->id == bpi->sysid && (sb = look->init(blkdev))) {
            *root = sb->root;
            return 0;
        }
    }
    return -ENODEV;
}
