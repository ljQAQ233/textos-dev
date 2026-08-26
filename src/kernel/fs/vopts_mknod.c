#include <textos/errno.h>
#include <textos/fs.h>

extern fs_opts_t __vfs_dev_op;
/*
 * if dev is a network device it also mknods, the precondition
 * is that it is called by the kernel... instead of mknod syscall!
 */
int vfs_mknod(char *path, dev_t dev, int mode)
{
    int ret;
    if (!S_ISCHR(mode) && !S_ISBLK(mode) && !S_ISFIFO(mode) && !S_ISSOCK(mode))
        return -EINVAL;

    node_t *nod;
    node_t *dir;
    struct fs_openctx ctx = {0};
    ret = vfs_walkd(NULL, &path, &dir);
    if (ret < 0) return ret;
    ret = vfs_open(dir, path, 0, 0, &nod, &ctx);
    if (ret >= 0) return -EEXIST;
    ret = dir->sb->op->mknod(dir, path, dev, mode, &nod);
    if (ret < 0) return ret;
    nod->opts = &__vfs_dev_op;
    return ret;
}
