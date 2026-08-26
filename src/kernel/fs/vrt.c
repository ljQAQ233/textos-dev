#include <textos/dev/buffer.h>
#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/fs/inter.h>
#include <textos/klib/string.h>
#include <textos/mm.h>
#include <textos/printk.h>

/*
   用来注册文件系统, 在这里列出的文件系统, 是系统支持的
*/
typedef struct
{
    char *name;
    int id;
    superblk_t *(*init)(devst_t *hd);
} regstr_t;

#include <textos/dev.h>

extern superblk_t *__fs_init_fat32(devst_t *dev);
extern superblk_t *__fs_init_minix(devst_t *dev);

// clang-format off

static regstr_t regstr[] = {
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

#include <textos/args.h>
#include <textos/klib/vsprintf.h>

static void _init_partitions(devst_t *hd, mbr_t *rec)
{
    part_t *ptr = rec->ptab;

    printk("Looking for file systems...\n");

    for (int i = 0, nr = 0; i < 4; i++, ptr++) {
        if (!ptr->sysid) continue;

        char *type = "none";
        node_t *root = NULL;
        superblk_t *sb = NULL;
        devst_t *dev = register_part(hd, nr++, ptr->relative, ptr->total, root);

        for (regstr_t *look = regstr; look->id != 0; look++) {
            if (look->id != ptr->sysid) continue;
            sb = look->init(dev);
            if (!sb) break;
            root = sb->root;
            dev->pdata = root;
            type = look->name;
            if (!__vfs_rootset(root))
                ; // is not root
        }

        printk(" - partition %u -> %s\n", i, type);
    }

    vfs_listnode(__vfs_root);
}

// todo: fix fat32_truncate

extern void __vfs_pipe_init();
extern node_t *__fs_init_tmpfs();
extern node_t *__fs_init_procfs();

void fs_init()
{
    devst_t *hd = dev_lookup_type(DEV_IDE, 0);
    buffer_t *recblk = bread(hd, 512, 0);
    _init_partitions(hd, recblk->blk);
    brelse(recblk);

    // abstract
    __vfs_pipe_init();

    vfs_mount_to("/tmp", __fs_init_tmpfs(),
                 S_IFDIR | S_IRWXG | S_IRWXU | S_IRWXO);
    vfs_mount_to("/dev", __fs_init_tmpfs(),
                 S_IFDIR | S_IRWXG | S_IRWXU | S_IRWXO);
    vfs_mount_to("/proc", __fs_init_procfs(),
                 S_IFDIR | S_IRGRP | S_IXGRP | S_IRUSR | S_IXUSR | S_IROTH |
                     S_IXOTH);

    printk("file system initialized!\n");
}
