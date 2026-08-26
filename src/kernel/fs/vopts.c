#include <textos/fs.h>

int vfs_read(node_t *this, void *buffer, size_t siz, size_t offset,
             struct fs_openctx *openctx)
{
    ASSERTK(openctx != NULL);
    return this->opts->read(this, buffer, siz, offset, openctx);
}

int vfs_write(node_t *this, void *buffer, size_t siz, size_t offset,
              struct fs_openctx *openctx)
{
    ASSERTK(openctx != NULL);
    return this->opts->write(this, buffer, siz, offset, openctx);
}

int vfs_close(node_t *this, struct fs_openctx *openctx)
{
    ASSERTK(openctx != NULL);
    if (this->opts->_fini_pctx) //
        this->opts->_fini_pctx(this, &openctx->pctx);
    return this->opts->close(this);
}

int vfs_truncate(node_t *this, size_t offset)
{
    return this->opts->truncate(this, offset);
}

int vfs_remove(node_t *this)
{
    return this->opts->remove(this);
}
