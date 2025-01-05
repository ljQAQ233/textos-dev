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

#define VFS_CREATE  0x01
#define VFS_DIR     0x02
#define VFS_GAIN    0x04
#define VFS_VRT     0x08

struct node;
typedef struct node node_t;

typedef struct {
    int  (*open)(node_t *parent, char *path, u64 args, node_t **result);
    int  (*close)(node_t *this);
    int  (*remove)(node_t *this);
    /* 文件操作 */
    int  (*read)(node_t *this, void *buf, size_t siz, size_t offset);
    int  (*write)(node_t *this, void *buf, size_t siz, size_t offset);
    int  (*truncate)(node_t *this, size_t offset);
    /* 文件夹操作 */
    int  (*readdir)(node_t *this, node_t **res, size_t idx);
} fs_opts_t;

struct node {
    char *name;

    u64 attr;
    u64 siz;     // Zero for dir

    // unused
    node_t *root;

    node_t *parent;
    node_t *child;
    node_t *next;

    void *sys;
    int systype;
    addr_t idx;
    void *pdata;

    /* Interfaces */
    fs_opts_t *opts;
};

extern int vfs_open (node_t *parent, node_t **node, const char *path, u64 args);

extern int vfs_read (node_t *this, void *buf, size_t siz, size_t offset);

extern int vfs_write (node_t *this, void *buf, size_t siz, size_t offset);

extern int vfs_close (node_t *this);

extern int vfs_remove (node_t *this);

extern int vfs_truncate (node_t *this, size_t offset);

extern int vfs_release (node_t *this);

extern int vfs_readdir (node_t *this, node_t **res, size_t idx);

extern node_t *vfs_test (node_t *start, char *path, node_t **last, char **lastpath);

extern void vfs_initops (fs_opts_t *opts);

#endif
