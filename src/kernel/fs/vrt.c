#include <textos/mm.h>
#include <textos/fs.h>
#include <textos/printk.h>
#include <textos/fs/inter.h>
#include <textos/panic.h>
#include <textos/assert.h>
#include <textos/errno.h>

#include <string.h>

/*
   用来注册文件系统, 在这里列出的文件系统, 是系统支持的
*/
typedef struct {
    char *name;
    int id;
    superblk_t *(*init)(devst_t *hd);
} regstr_t;

static node_t *_fs_root = NULL;

bool __vfs_rootset (node_t *root)
{
    if (!_fs_root)
        _fs_root = root;
    return _fs_root == root;
}

_UTIL_NEXT();

static bool _cmp (char *A, char *B)
{
    size_t lenA = MIN (strlen(A), strchr (A, '/') - A);
    size_t lenB = MIN (strlen(B), strchr (B, '/') - B);

    if (lenA != lenB)
        return false;

    for (size_t i = 0 ; i < lenB ; i++)
        if (A[i] != B[i])
            return false;

    return true;
}

node_t *vfs_test (node_t *start, char *path, node_t **node_last, char **path_last)
{
    node_t *res = NULL,
           *curr = start;

    for ( ; ; ) {
        for (node_t *ptr = curr->child ;  ; ptr = ptr->next) {
            if (!ptr)
                goto fini;

            if (_cmp(ptr->name, path)) {
                char *nxt = _next(path);
                if (nxt[0] == 0) {
                    res = ptr;
                    goto fini;
                }
                path = nxt;
                curr = ptr;
                break;
            } else {
                continue;
            }
        }
    }

fini:
    if (node_last)
        *node_last = curr;
    if (path_last)
        *path_last = path;
    
    return res;
}

node_t *vfs_exist(node_t *dir, char *path)
{
    for (node_t *ptr = dir->child ; ptr ; ptr = ptr->next) {
        if (_cmp(ptr->name, path))
            return ptr;
    }

    return NULL;
}

void vfs_initops(fs_opts_t *opts)
{
    opts->open = noopt;
    opts->ioctl = noopt;
    opts->close = noopt;
    opts->remove = noopt;
    opts->read = noopt;
    opts->write = noopt;
    opts->truncate = noopt;
    opts->readdir = noopt;
    opts->mmap = noopt;
}

void vfs_regst(node_t *n, node_t *p)
{
    n->next = p->child;
    p->child = n;
    n->parent = p;
}

void vfs_unreg(node_t *n)
{
    node_t **pp = &n->parent->child;
    while (*pp && *pp != n)
        pp = &(*pp)->next;
    if (*pp == n)
        *pp = n->next;
}

node_t *vfs_getprt(node_t *n)
{
    n = n->parent;
    if (vfs_ismount(n))
        n = n->parent;
    return n;
}

/*
 * get mount point's node if n is a mount point
 */
static node_t *getprt_mp(node_t *n)
{
    ASSERTK(!vfs_isaroot(n));
    n = n->parent;
    if (vfs_isaroot(n))
        n = n->parent;
    return n;
}

int vfs_getpath(node_t *n, char *buf, size_t *size)
{
    if (n == NULL)
    {
        if (*size < 2)
        {
            *size = 2;
            return -ERANGE;
        }
        buf[0] = '/';
        buf[1] = 0;
        *size = 2;
        return 0;
    }
    if (vfs_isaroot(n))
        n = n->parent;

    node_t *p;
    int len = 0;
    int toklen;
    for (p = n ; p != p->parent ; p = getprt_mp(p))
        len += strlen(p->name) + 1;
    
    if (*size < len + 1)
    {
        *size = len + 1;
        return -ERANGE;
    }

    char *end = buf + len;
    *end = 0;

    for (p = n ; p != p->parent ; p = getprt_mp(p))
    {
        toklen = strlen(p->name);
        end -= toklen;
        memcpy(end, p->name, toklen);
        *(--end) = '/';
    }
    ASSERTK(end == buf);
    *size = len + 1;
    return 0;
}

static int _vfs_open (node_t *dir, node_t **node, char *path, u64 args, int mode)
{
    int ret = 0;
    node_t *res;

    if (_cmp(path, ".")) {
        res = dir;
        goto fini;
    } else if (_cmp(path, "..")) {
        res = vfs_getprt(dir);
        goto fini;
    }

    res = vfs_exist(dir, path);
    if (res == NULL) {
        ret = dir->opts->open (dir, path, args, mode, &res);
        if (ret < 0) {
            res = NULL;
            goto fini;
        }
    }

    if (vfs_ismount(res))
    {
        if (~args & FS_GAINMNT)
            res = res->child;
    }

fini:
    *node = res;

    return ret;
}

static int _vfs_walk (node_t *start, node_t **node, char *path, u64 args, int mode)
{
    /* 为 Open 扫一些障碍! */
    if (!start) start = _fs_root;
    if (path[0] == '/') start = _fs_root;
    while (*path == '/') path++;

    node_t *cur = start;
    for ( ; ; )
    {
        if (!path[0])
            break;

        char *nxt = _next(path);
        node_t *chd;
        int ret;
        if (!nxt[0]) {
            ret = _vfs_open(cur, &chd, path, args, mode);
        } else {
            if (!S_ISDIR(cur->mode))
                return -ENOTDIR;
            ret = _vfs_open(cur, &chd, path, FS_GAIN, mode);
        }

        if (ret < 0)
            return ret;
        
        cur = chd;
        path = nxt;
    }

    *node = cur;
    return 0;
}

int vfs_open (node_t *parent, node_t **node, const char *path, u64 args, int mode)
{
    DEBUGK(K_FS, "try to open %s\n", path);

    int ret = _vfs_walk (parent, node, (char *)path, args, mode);
    node_t *opened = *node;
    if (opened && !(args & FS_GAIN)) {
        if (!S_ISDIR(opened->mode) && args & O_DIRECTORY)
            ret = -ENOTDIR;
        else if (S_ISDIR(opened->mode) && ~args & O_DIRECTORY)
            ret = -EISDIR;
    }
    if (ret < 0)
        DEBUGK(K_FS, "failed to open %s (%#x) - ret : %d\n", path, args, ret);

    return ret;
}

int vfs_read (node_t *this, void *buffer, size_t siz, size_t offset)
{
    int ret = this->opts->read (this, buffer, siz, offset);
    if (ret < 0)
        DEBUGK(K_FS, "failed to read %s - ret : %d\n", this->name, ret);

    return ret;
}
    
int vfs_write (node_t *this, void *buffer, size_t siz, size_t offset)
{
    int ret = this->opts->write (this, buffer, siz, offset);
    if (ret < 0)
        DEBUGK(K_FS, "failed to write %s - ret : %d\n", this->name, ret);

    return ret;
}

int vfs_close (node_t *this)
{
    int ret = this->opts->close (this);
    if (ret < 0)
        DEBUGK(K_FS, "failed to close %s - ret : %d\n", this->name, ret);

    return ret;
}

int vfs_remove (node_t *this)
{
    int ret = this->opts->remove (this);
    if (ret < 0)
        DEBUGK(K_FS, "failed to remove %s - ret : %d\n", this->name, ret);
    return ret;
}

int vfs_truncate (node_t *this, size_t offset)
{
    int ret = this->opts->truncate (this, offset);
    if (ret < 0)
        DEBUGK(K_FS, "failed to truncate %s - ret : %d\n", this->name, ret);
    return ret;
}

/*
 * FIXME: do not use this after a close directly!!! page fault may happen
 *        for instance, you cannot release a mount directory or even the vfs root!
 */
int vfs_release (node_t *this)
{
    if (S_ISDIR(this->mode))
        while (this->child)
            vfs_release (this->child);

    /* 除去父目录项的子目录项 */
    if (this->parent)
        vfs_unreg(this);

    /* 释放信息 */
    if (this->name)
        free(this->name);

fini:
    free(this);
    return 0;
}

extern fs_opts_t __vfs_devop;

int vfs_mknod (char *path, dev_t dev, int mode)
{
    int ret;
    if (!S_ISCHR(mode)
     && !S_ISBLK(mode)
     && !S_ISFIFO(mode))
        return -EINVAL;
    
    node_t *node;
    ret = vfs_open(NULL, &node, path, FS_MKNOD | O_CREAT, mode);
    if (ret < 0)
        return ret;
    unsigned ma = major(dev);
    unsigned mi = minor(dev);
    node->mode = mode;
    node->rdev = dev;
    node->pdata = dev_lookup_nr(ma, mi);
    node->opts = &__vfs_devop;

    return ret;
}

static inline void _vfs_listnode (node_t *node, int level)
{
    if (S_ISDIR(node->mode)) {
        printk ("%*q- %s\n", level, ' ', node->name);
        for (node_t *p = node->child ; p != NULL ; p = p->next) {
            _vfs_listnode (p, level + 1);
        }
    } else {
        printk ("%*q- %s\n", level, ' ', node->name);
    }
}

void __vfs_listnode (node_t *start)
{
    if (!start)
        start = _fs_root;

    _vfs_listnode (start, 0);
}

#include <textos/dev.h>

extern FS_INITIALIZER(__fs_init_fat32);

// clang-format off

static regstr_t regstr[] = {
    [FS_FAT32] = {
        .name = "fat32",
        .id = 0xc,
        .init = __fs_init_fat32
    },
    {
        .name = "endsym",
        .id = 0,
        .init = NULL
    }
};

// clang-format on

struct pub {
    devst_t *dev;
    devst_t *devp;
    node_t *root;
};

#include <textos/args.h>
#include <textos/klib/vsprintf.h>

static void _init_partitions (devst_t *hd, mbr_t *rec)
{
    part_t *ptr = rec->ptab;

    printk ("Looking for file systems...\n");

    for (int i = 0, nr = 0 ; i < 4 ; i++, ptr++) {
        if (!ptr->sysid)
            continue;

        char *type = "none";
        node_t *root = NULL;
        superblk_t *sb = NULL;
        devst_t *dev = register_part(hd, nr++,
                ptr->relative,
                ptr->total, root
                );

        for (regstr_t *look = regstr; look->id != 0 ; look++) {
            if (look->id != ptr->sysid)
                continue;
            sb = look->init(dev);
            if (!sb)
                break;
            root = sb->root;
            dev->pdata = root;
            type = look->name;
            if (!__vfs_rootset(root))
                ; // is not root
        }

        printk (" - partition %u -> %s\n", i, type);
    }

    __vfs_listnode (_fs_root);
}

// todo: fix fat32_truncate

extern void __pipe_init();
extern void __kconio_init();
extern void __vrtdev_init();
extern node_t *__fs_init_tmpfs();
extern node_t *__fs_init_procfs();

#include <textos/mm/vmm.h>

void fs_init ()
{
    devst_t *hd = dev_lookup_type (DEV_IDE, 0);

    mbr_t *record = vmm_allocpages(1, PE_P | PE_RW);
    hd->bread (hd, 0, record, 1);

    _init_partitions (hd, record);

    free(record);

    // abstract
    __pipe_init();
    __kconio_init();
    __vrtdev_init();

    vfs_mount_to("/tmp", __fs_init_tmpfs(), S_IFDIR | S_IRWXG | S_IRWXU | S_IRWXO);
    vfs_mount_to("/proc", __fs_init_procfs(), S_IFDIR | S_IRGRP | S_IXGRP | S_IRUSR | S_IXUSR | S_IROTH | S_IXOTH);

    printk ("file system initialized!\n");
}

