#include <textos/fs/inter.h>

_UTIL_CMP();

node_t *vfs_exist(node_t *dir, char *path)
{
    for (node_t *ptr = dir->child; ptr; ptr = ptr->next)
        if (_cmp(ptr->name, path)) return ptr;
    return NULL;
}

void vfs_initops(fs_opts_t *op)
{
    op->open = noopt;
    op->close = noopt;
    op->_init_pctx = NULL;
    op->_fini_pctx = NULL;
    op->mknod = noopt;
    op->chown = noopt;
    op->chmod = noopt;
    op->remove = noopt;
    op->truncate = noopt;
    op->readdir = noopt;
    op->seekdir = noopt;
    op->mmap = noopt;
    op->read = noopt;
    op->write = noopt;
    op->ioctl = noopt;
}

void vfs_regst(node_t *n, node_t *p)
{
    n->next = p->child;
    p->child = n;
    n->parent = p;
    list_init(&n->r_ctx_all);
}

void vfs_unreg(node_t *n)
{
    node_t **pp = &n->parent->child;
    while (*pp && *pp != n)
        pp = &(*pp)->next;
    if (*pp == n) *pp = n->next;
}

node_t *vfs_getprt(node_t *n)
{
    n = n->parent;
    if (vfs_ismount(n)) n = n->parent;
    return n;
}

/*
 * FIXME: do not use this after a close directly!!! page fault may happen
 *        for instance, you cannot release a mount directory or even the vfs
 * root!
 */
int vfs_release(node_t *this)
{
    if (S_ISDIR(this->mode))
        while (this->child)
            vfs_release(this->child);

    /* 除去父目录项的子目录项 */
    if (this->parent) vfs_unreg(this);

    /* 释放信息 */
    if (this->name) free(this->name);

    free(this);
    return 0;
}

static inline void _vfs_listnode(node_t *node, int level)
{
    if (S_ISDIR(node->mode)) {
        dprintk("%*q- %s\n", level, ' ', node->name);
        for (node_t *p = node->child; p != NULL; p = p->next) {
            _vfs_listnode(p, level + 1);
        }
    } else {
        dprintk("%*q- %s\n", level, ' ', node->name);
    }
}

void vfs_listnode(node_t *start)
{
    if (!start) start = __vfs_root;

    _vfs_listnode(start, 0);
}
