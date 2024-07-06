#include <textos/mm.h>
#include <textos/fs.h>
#include <textos/printk.h>
#include <textos/fs/inter.h>
#include <textos/debug.h>
#include <textos/assert.h>

#include <string.h>

/*
   用来注册文件系统, 在这里列出的文件系统, 是系统支持的
*/
typedef struct {
    char      *name;
    u8        id;
    void      *(*init)(dev_t *hd, mbr_t *mbr, part_t *pentry);
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

node_t *__vfs_test (node_t *start, char *path, node_t **node_last, char **path_last)
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

/* return-value for interfaces */
#include <textos/errno.h>

static int _vfs_open (node_t *parent, node_t **node, char *path, u64 args)
{
    /* 为 Open 扫一些障碍! */
    if (!parent) parent = _fs_root;
    if (path[0] == '/') parent = _fs_root;
    while (*path == '/') path++;

    int ret = 0;
    node_t *res;
    node_t *start;
    if ((res = __vfs_test (parent, path, &start, &path)))
        goto fini;

    ret = start->opts->open (start, path, args, &res);
    if (ret < 0) {
        res = NULL;
        goto fini;
    }

    /* TODO: Permission checking */

fini:
    *node = res;

    return ret;
}

int __vfs_open (node_t *parent, node_t **node, const char *path, u64 args)
{
    ASSERTK (!parent || CKDIR(parent));

    int ret = _vfs_open (parent, node, (char *)path, args);
    if (ret < 0)
        DEBUGK(K_FS, "failed to open %s (%#x) - ret : %d\n", path, args, ret);

    return ret;
}

int __vfs_read (node_t *this, void *buffer, size_t siz, size_t offset)
{
    ASSERTK (CKFILE(this));

    int ret = this->opts->read (this, buffer, siz, offset);
    if (ret < 0)
        DEBUGK(K_FS, "failed to read %s - ret : %d\n", this->name, ret);

    return ret;
}
    
int __vfs_write (node_t *this, void *buffer, size_t siz, size_t offset)
{
    ASSERTK (CKFILE(this));

    int ret = this->opts->write (this, buffer, siz, offset);
    if (ret < 0)
        DEBUGK(K_FS, "failed to write %s - ret : %d\n", this->name, ret);

    return ret;
}

int __vfs_close (node_t *this)
{
    int ret = this->opts->close (this);
    if (ret < 0)
        DEBUGK(K_FS, "failed to close %s - ret : %d\n", this->name, ret);

    return ret;
}

int __vfs_remove (node_t *this)
{
    int ret = this->opts->remove (this);
    if (ret < 0)
        DEBUGK(K_FS, "failed to remove %s - ret : %d\n", this->name, ret);
    return ret;
}

int __vfs_truncate (node_t *this, size_t offset)
{
    ASSERTK (CKFILE(this));

    int ret = this->opts->truncate (this, offset);
    if (ret < 0)
        DEBUGK(K_FS, "failed to truncate %s - ret : %d\n", this->name, ret);
    return ret;
}

int __vfs_release (node_t *this)
{
    if (this->attr & NA_DIR)
        while (this->child)
            __vfs_release (this->child);

    /* 除去父目录项的子目录项 */
    if (this->parent)
    {
        node_t *curr = this->parent->child;
        node_t *prev = this->parent->child;

        while (curr != NULL) {
            curr = curr->next;
            if (curr == this) {
                prev->next = curr->next;
                break;
            }
            prev = prev->next;
        } 
    }

    /* 释放信息 */
    if (this->name)
        free(this->name);

fini:
    free(this);
    return 0;
}

int __vfs_readdir (node_t *this)
{
    ASSERTK (!this || CKDIR(this));
    if (!this) this = _fs_root;

    int ret = this->opts->readdir (this);
    if (ret < 0)
        DEBUGK(K_FS, "read directory failed - ret : %d\n", ret);

    return ret;
}

static inline void _vfs_listnode (node_t *node, int level)
{
    if (node->attr & NA_DIR) {
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

extern FS_INITIALIZER ( __fs_init_fat32);

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

static void _init_partitions (dev_t *hd, mbr_t *rec)
{
    part_t *ptr = rec->ptab;

    printk ("Looking for file systems...\n");

    for (int i = 0 ; i < 4 ; i++, ptr++) {
        if (!ptr->sysid)
            continue;

        char *type = "none";
        void *root = NULL;

        for (regstr_t *look = regstr; look->id != 0 ; look++) {
            if (look->id == ptr->sysid)
                if ((root = look->init (hd, rec, ptr)))
                {
                    type = look->name;

                    __vfs_rootset (root);
                }
        }

        printk (" - partition %u -> %s\n", i, type);
    }

    __vfs_listnode (_fs_root);
}

void fs_init ()
{
    dev_t *hd = dev_lookup_type (DEV_BLK, DEV_IDE);

    mbr_t *record = malloc(sizeof(mbr_t));
    hd->bread (hd, 0, record, 1);

    _init_partitions (hd, record);

    free(record);

    printk ("file system initialized!\n");
    
    node_t *file, *dir;
    
    char buf[1024] = "Hello world!";
    __vfs_open (NULL, &dir, "/TEST", O_READ | O_CREATE | O_DIR);
    __vfs_open (NULL, &file, "/TEST/test.txt", O_READ | O_CREATE);
    __vfs_write (file, buf, 12, 0);
    __vfs_truncate (file, 10000);
    __vfs_readdir (NULL);
    __vfs_listnode (NULL);
}

