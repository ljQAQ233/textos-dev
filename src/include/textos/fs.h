#ifndef __FILE_SYS_H__
#define __FILE_SYS_H__

enum
{
    FS_FAT32,
    FS_END,
};

/*
 * vfs attributes
 */
#define FSA_MNT (1 << 0)

/*
 * structure of open flgs:
 *   - bits 0 ~ 31 - posix / system specified flags passed from user space
 *   - bits 32 ~ 63 - used by vfs, specifying special operations e.g. ignore errors
 */
#define FS_GAIN    (1ull << 32) // ignore checks for dir / file, ignoring EISDIR / ENOTDIR
#define FS_GAINMNT (1ull << 33) // open dir mounted to, not root dir of mountpoint
#define FS_MKNOD   (1ull << 34) // mknod operation, used by physical fs. may be ignored

struct node;
typedef struct node node_t;

struct dirctx;
typedef struct dirctx dirctx_t;

#include <textos/dev.h>
#include <textos/file.h>
#include <textos/time.h>
#include <textos/noopt.h>
#include <textos/mm/mman.h>

typedef struct
{
    int  (*open)(node_t *parent, char *path, u64 args, int mode, node_t **result);
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

struct node
{
    char *name;

    u64 attr;
    u64 siz;     // Zero for dir
    mode_t mode;
    time_t atime;
    time_t mtime;
    time_t ctime;

    // unused
    node_t *root;

    node_t *parent;
    node_t *child;
    node_t *next;

    dev_t dev;
    dev_t rdev;
    void *sys;
    int systype;
    addr_t idx;
    void *pdata;
    void *mount;

    /* Interfaces */
    fs_opts_t *opts;
};

enum
{
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

int vfs_open (node_t *parent, node_t **node, const char *path, u64 args, int mode);
int vfs_read(node_t *this, void *buf, size_t siz, size_t offset);
int vfs_write(node_t *this, void *buf, size_t siz, size_t offset);
int vfs_close(node_t *this);
int vfs_remove(node_t *this);
int vfs_truncate(node_t *this, size_t offset);
int vfs_release(node_t *this);
int vfs_readdir(node_t *this, node_t **res, size_t idx);

#include <textos/dev.h>

int vfs_mknod(char *path, dev_t dev, int mode);

node_t *vfs_test(node_t *start, char *path, node_t **last, char **lastpath);

void vfs_initops(fs_opts_t *opts);

int vfs_mount(node_t *dir, node_t *root);
int vfs_mount_to(char *path, node_t *root, int mode);
int vfs_umount(node_t *dir);

#endif
