#include <textos/boot.h>
#include <textos/dev.h>
#include <textos/fs.h>
#include <textos/fs/internal.h>
#include <textos/fs/mount.h>

extern bool __vfs_rootset(node_t *root);

extern node_t *__fs_init_tmpfs();
extern node_t *__fs_init_procfs();

static devst_t *lookup_fakepath(const char *fake)
{
    const char *p = fake;
    if (strncmp(p, "/dev/", 5) == 0) p += 5;
    return dev_lookup_name(p);
}

void initproc_mnt()
{
    dev_debug_list();
    char *initrd = 0;
    node_t *sysroot;
    if (initrd) {
        sysroot = __fs_init_tmpfs();
        if (!sysroot) PANIC("temporary rootfs cannot be initialized\n");
    } else {
        char *name = BOOT_DEV_PATH_BOOT;
        devst_t *blkdev = lookup_fakepath(name);
        if (!blkdev) PANIC("root device (%s) not found\n", name);
        int ret = fs_extract_mount(blkdev, &sysroot);
        if (ret < 0) PANIC("no partition recognized on %s\n", name);
    }
    __vfs_rootset(sysroot);

    vfs_mount_to("/tmp", __fs_init_tmpfs(),
                 S_IFDIR | S_IRWXG | S_IRWXU | S_IRWXO);
    vfs_mount_to("/dev", __fs_init_tmpfs(),
                 S_IFDIR | S_IRWXG | S_IRWXU | S_IRWXO);
    vfs_mount_to("/proc", __fs_init_procfs(),
                 S_IFDIR | S_IRGRP | S_IXGRP | S_IRUSR | S_IXUSR | S_IROTH |
                     S_IXOTH);
}
