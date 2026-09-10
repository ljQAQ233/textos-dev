#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/mm/heap.h>

int vfs_symlink(char *path, char *linkto, node_t **result)
{
    int ret;
    node_t *dir = NULL;

    ret = vfs_walkd(NULL, &path, &dir);
    if (ret < 0) goto fail;

    ret = dir->opts->symlink(dir, path, linkto, result);
fail:
    return ret;
}

int vfs_readlink(node_t *this, char *linkto, size_t *siz)
{
    ASSERTK(siz != NULL);
    return this->opts->readlink(this, linkto, siz);
}

int vfs_readlink_auto(node_t *this, char **linkto, size_t *siz)
{
    int ret;
    char *buf = NULL;
    size_t bufsz = 0;

    ret = this->opts->readlink(this, buf, &bufsz);
    if (ret != -ENAMETOOLONG) {
        goto fail_getsize;
    }

    buf = malloc(bufsz + 1);
    if (!buf) {
        ret = -ENOMEM;
        goto fail_malloc;
    }
    ret = this->opts->readlink(this, buf, &bufsz);
    if (ret >= 0) buf[ret] = '\0';

fail_malloc:
    *linkto = buf;
    *siz = bufsz;
fail_getsize:
    return ret;
}
