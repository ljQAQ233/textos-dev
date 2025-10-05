#ifndef __FILE_SYS_H__
#define __FILE_SYS_H__

enum
{
    FS_FAT32,
    FS_MINIX1,
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

#include <textos/type.h>
#include <textos/dev.h>
#include <textos/file.h>
#include <textos/time.h>
#include <textos/noopt.h>
#include <textos/mm/mman.h>

typedef struct
{
    /*
     * dir entry operations
     */
    int  (*open)(node_t *parent, char *path, u64 args, int mode, node_t **result);
    /*
     * some fs do not support device node. here we create an empty file and bind
     * it with a device only in memory (vfs node_t::rdev). it is certain that
     * `name` is not exist (filtered by vfs before)
     */
    int  (*mknod)(node_t *parent, char *name, dev_t rdev, int mode, node_t **result);
    /*
     * vfs layer handles -1, physical fs gets final uid/gid only. physical fs must update
     * both node's or on-disk inode's ctime, uid/gid and clear SUID bit and SGID bit
     * according to `ap` (appropriate privilege) flag after a successful modification.
     */
    int  (*chown)(node_t *this, uid_t owner, gid_t group, bool ap);
    int  (*chmod)(node_t *this, mode_t mode, bool clrsgid);
    int  (*remove)(node_t *this);
    int  (*readdir)(node_t *this, dirctx_t *ctx);
    int  (*seekdir)(node_t *this, dirctx_t *ctx, size_t *pos);

    /*
     * file operations
     */
    int  (*read)(node_t *this, void *buf, size_t siz, size_t offset);
    int  (*write)(node_t *this, void *buf, size_t siz, size_t offset);
    int  (*truncate)(node_t *this, size_t offset);
    void *(*mmap)(node_t *this, vm_region_t *vm);
    int  (*ioctl)(node_t *this, int req, void *argp);
    int  (*close)(node_t *this);
} fs_opts_t;

struct node
{
    char *name;

    u64 attr;
    u64 siz;
    ino_t ino;
    uid_t uid;
    gid_t gid;
    mode_t mode;
    time_t atime;
    time_t mtime;
    time_t ctime;
    
    dev_t dev;
    dev_t rdev;

    /*
     * a dir with FSA_MNT uses it to store history directory entries which
     * is attached to this node. Actually a stack in implementation,
     * mount overlay as known
     */
    void *mount;

    void *pdata;
    superblk_t *sb;
    fs_opts_t *opts;

    node_t *parent;
    node_t *child;
    node_t *next;
};

struct superblk
{
    int fstype;
    blksize_t blksz;
    devst_t *dev;
    node_t *root;
    fs_opts_t *op;
    void *sbi;
};

enum
{
    ctx_inv = 0, // invalid
    ctx_pre,     // prepared
    ctx_end,     // end of dir
};

struct dirctx
{
    superblk_t *sb;
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

void vfs_regst(node_t *n, node_t *p);
void vfs_unreg(node_t *n);
node_t *vfs_getprt(node_t *n);

/**
 * @brief get null-terminated absolute path of n
 * 
 * @param n    node
 * @param buf  buffer
 * @param size on input, the size of buffer passed into
 *             on output, size is the minimum size needed to store the path
 * @return int size (including '\0') written to buf or an errno
 */
int vfs_getpath(node_t *n, char *buf, size_t *size);

#define MAY_READ  (1 << 0)
#define MAY_WRITE (1 << 1)
#define MAY_EXEC  (1 << 2)

int vfs_permission(node_t *n, int want);

int vfs_open(node_t *parent, const char *path, u64 args, int mode, node_t **result);
int vfs_read(node_t *this, void *buf, size_t siz, size_t offset);
int vfs_write(node_t *this, void *buf, size_t siz, size_t offset);
int vfs_close(node_t *this);
int vfs_remove(node_t *this);
int vfs_truncate(node_t *this, size_t offset);
int vfs_chown(node_t *file, uid_t owner, gid_t group);
int vfs_chmod(node_t *file, mode_t mode);
int vfs_release(node_t *this);
int vfs_readdir(node_t *this, node_t **res, size_t idx);

#include <textos/dev.h>

int vfs_mknod(char *path, dev_t dev, int mode);

node_t *vfs_test(node_t *start, char *path, node_t **last, char **lastpath);

void vfs_initops(fs_opts_t *opts);

int vfs_mount(node_t *dir, node_t *root);
int vfs_mount_to(char *path, node_t *root, int mode);
int vfs_umount(node_t *dir);

/**
 * @brief is n a mount point whose `child` is a fs root?
 */
bool vfs_ismount(node_t *n);

/**
 * @brief is n a mounted root of a fs? (excluding vfs' root)
 */
bool vfs_isaroot(node_t *n);

#endif
