#include <textos/fs.h>
#include "../fs/internal.h"
    
extern bool __vfs_rootset(node_t *root);

extern node_t *__fs_init_tmpfs();
extern node_t *__fs_init_procfs();

void initproc_mnt()
{
    __vfs_rootset(__fs_init_tmpfs());
    vfs_mount_to("/tmp", __fs_init_tmpfs(),
                 S_IFDIR | S_IRWXG | S_IRWXU | S_IRWXO);
    vfs_mount_to("/dev", __fs_init_tmpfs(),
                 S_IFDIR | S_IRWXG | S_IRWXU | S_IRWXO);
    vfs_mount_to("/proc", __fs_init_procfs(),
                 S_IFDIR | S_IRGRP | S_IXGRP | S_IRUSR | S_IXUSR | S_IROTH |
                     S_IXOTH);
}
