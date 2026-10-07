#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/task.h>

extern fs_opts_t __vfs_dev_op;

/*
 * if dev is a network device it also mknods, the precondition
 * is that it is called by the kernel... instead of mknod syscall!
 */
int vfs_mknod(const char *path, dev_t dev, int mode)
{
    assert(S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode) || S_ISSOCK(mode));
    int ret;
    node_t *nod;
    node_t *dir;
    struct fs_openctx ctx = {0};
    ret = vfs_walkd(task_current()->pwd, &path, &dir);
    if (ret < 0) return ret;
    ret = vfs_open(dir, path, 0, 0, &nod, &ctx);
    if (ret >= 0) return -EEXIST;
    ret = dir->sb->op->mknod(dir, path, dev, mode, &nod);
    if (ret < 0) return ret;
    nod->opts = &__vfs_dev_op;
    return ret;
}
