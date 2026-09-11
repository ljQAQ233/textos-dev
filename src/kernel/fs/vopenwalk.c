#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/fs/internal.h>

static int _vfs_open(node_t *dir, node_t **node, const char *path, u64 args,
                     int mode)
{
    int ret = 0;
    node_t *res;

    if (!path[0] || fs_name_cmp(path, ".")) {
        res = dir;
        goto fini;
    } else if (fs_name_cmp(path, "..")) {
        res = vfs_getprt(dir);
        goto fini;
    }

    res = vfs_exist(dir, path);
    if (!res) {
        ret = dir->sb->op->open(dir, path, args, mode, &res);
        if (ret < 0) goto fini;
    }

    if (vfs_ismount(res)) {
        if (~args & FS_GAINMNT) {
            res = res->child;
        }
    }
    if (S_ISLNK(res->mode) && !(args & FS_GAINLNK)) {
        char *linkto;
        size_t linkto_len;
        struct fs_openctx fakectx;
        ret = vfs_readlink_auto(res, &linkto, &linkto_len);
        if (ret < 0) goto fini;
        ret = vfs_open(dir, linkto, FS_GAIN, 0, &res, &fakectx);
        free(linkto);
        if (ret < 0) goto fini;
    }

fini:
    *node = ret < 0 ? NULL : res;
    return ret;
}

int vfs_walkd(node_t *start, const char **path, node_t **node)
{
    const char *p = *path;
    if (!start) start = __vfs_root;
    if (p[0] == '/') start = __vfs_root;
    while (*p == '/')
        p++;
    int ret = 0;
    node_t *cur = start;
    for (;;) {
        char *nxt = fs_path_next(p);
        node_t *chd;
        if (!nxt[0]) break;
        if (!S_ISDIR(cur->mode)) {
            ret = -ENOTDIR;
            goto end;
        }
        if ((ret = vfs_permission(cur, MAY_EXEC)) < 0) goto end;
        ret = _vfs_open(cur, &chd, p, FS_GAIN, 0);
        if (ret < 0) goto end;

        cur = chd;
        p = nxt;
    }

end:
    *path = p;
    *node = cur;
    return 0;
}

int vfs_open(node_t *parent, const char *path, u64 args, int mode,
             node_t **node, struct fs_openctx *openctx)
{
    int ret = 0;
    const char *p = path;
    node_t *dir = NULL;
    node_t *res = NULL;
    ret = vfs_walkd(parent, &p, &dir);
    if (ret < 0) goto end;
    ret = _vfs_open(dir, &res, p, args, mode);
    if (ret < 0) goto end;
    if (res && !(args & FS_GAIN)) {
        int want = 0;
        switch (args & O_ACCMODE) {
        case O_RDONLY:
            want = MAY_READ;
            break;
        case O_WRONLY:
            want = MAY_WRITE;
            break;
        case O_RDWR:
            want = MAY_READ | MAY_WRITE;
            break;
        }
        if ((ret = vfs_permission(res, want)) < 0) goto end;
        if (!S_ISDIR(res->mode) && args & O_DIRECTORY)
            ret = -ENOTDIR;
        else if (S_ISDIR(res->mode) && !(args & O_DIRECTORY))
            ret = -EISDIR;
        if (S_ISLNK(dir->mode) && (args & O_NOFOLLOW)) {
            // If path names a symbolic link, fail and set errno to [ELOOP]
            ret = -ELOOP;
        }
    }
end:
    if (!ret) {
        *node = res;
        ASSERTK(openctx != NULL);
        openctx->file_flgs = args;
        openctx->pctx = NULL;
        list_init(&openctx->r_pollers);
        if (res->opts->_init_pctx) {
            ret = res->opts->_init_pctx(res, &openctx->pctx);
            if (ret < 0) *node = NULL;
            list_insert(&res->r_ctx_all, &openctx->l_ctx_all);
        }
    }
    DEBUGK(K_INFO, "open %s = %d\n", path, ret);
    return ret;
}
