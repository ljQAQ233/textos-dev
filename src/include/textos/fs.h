#ifndef __FILE_SYS_H__
#define __FILE_SYS_H__

enum {
    FS_FAT32,
    FS_END,
};

#define NA_DIR  (1 << 0)
#define NA_REG  (1 << 1)
#define NA_VRT  (1 << 2)
#define NA_DEV  (1 << 3)
#define NA_MNT  (1 << 4)

#define VFS_CREATE  0x01
#define VFS_DIR     0x02
#define VFS_GAIN    0x04
#define VFS_VRT     0x08
#define VFS_GAINMNT 0x10

struct node;
typedef struct node node_t;

struct dirctx;
typedef struct dirctx dirctx_t;

#include <textos/time.h>
#include <textos/noopt.h>
#include <textos/mm/mman.h>

typedef struct {
    int  (*open)(node_t *parent, char *path, u64 args, node_t **result);
    int  (*ioctl)(node_t *this, int req, void *argp);
    int  (*close)(node_t *this);
    int  (*remove)(node_t *this);
    /* 文件操作 */
    int  (*read)(node_t *this, void *buf, size_t siz, size_t offset);
    int  (*write)(node_t *this, void *buf, size_t siz, size_t offset);
    int  (*truncate)(node_t *this, size_t offset);
    /* 文件夹操作 */
    int  (*readdir)(node_t *this, dirctx_t *ctx);
    int  (*seekdir)(node_t *this, dirctx_t *ctx, size_t *pos);
    /* 文件映射 */
    void *(*mmap)(node_t *this, vm_region_t *vm);
} fs_opts_t;

struct node {
    char *name;

    u64 attr;
    u64 siz;     // Zero for dir
    time_t atime;
    time_t mtime;
    time_t ctime;

    // unused
    node_t *root;

    node_t *parent;
    node_t *child;
    node_t *next;

    void *sys;
    int systype;
    addr_t idx;
    void *pdata;
    void *mount;

    /* Interfaces */
    fs_opts_t *opts;
};

enum {
    ctx_inv = 0, // invalid
    ctx_pre,     // prepared
    ctx_end,     // end of dir
};

struct dirctx
{
    void *sys;
    node_t *node;
    int stat;
    size_t pos;
    addr_t bidx;
    addr_t eidx;
    size_t perblk;
    
    // emit info, DO NOT CHANGE IT EXCEPT
    // IN __readdir AND FS readdir !!!
    void *buf;
    size_t bufmx;
    size_t bufused;
    size_t bufents;
};

extern int vfs_open (node_t *parent, node_t **node, const char *path, u64 args);

extern int vfs_read (node_t *this, void *buf, size_t siz, size_t offset);

extern int vfs_write (node_t *this, void *buf, size_t siz, size_t offset);

extern int vfs_close (node_t *this);

extern int vfs_remove (node_t *this);

extern int vfs_truncate (node_t *this, size_t offset);

extern int vfs_release (node_t *this);

extern int vfs_readdir (node_t *this, node_t **res, size_t idx);

#include <textos/dev.h>

extern int vfs_mknod (char *path, devst_t *dev);

extern node_t *vfs_test (node_t *start, char *path, node_t **last, char **lastpath);

extern void vfs_initops (fs_opts_t *opts);

int vfs_mount(node_t *dir, node_t *root);

int vfs_mount_to(char *path, node_t *root);

int vfs_umount(node_t *dir);

#endif
