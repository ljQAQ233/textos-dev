#ifndef __FILE_H__
#define __FILE_H__

#include <textos/fs.h>
#include <textos/klib/list.h>

#define S_PIPE_R 0x01  // rx
#define S_PIPE_W 0x02  // tx

typedef struct
{
    size_t offset;
    node_t *node;
    dirctx_t *dirctx;
    int flgs;
    int fdfl;
    int refer;
    struct fs_openctx openctx;
} file_t;

int fd_get(int min);
int file_put(int fd);
int file_get(int *new, file_t **file, int min);

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

typedef struct
{
    int idx;
    int type;
    ino_t ino;
    size_t siz;
    char name[];
} dir_t;

#include <bits/perm.h>
#include <bits/mode.h>

typedef struct stat
{
    dev_t st_dev;
    ino_t st_ino;
    nlink_t st_nlink;
    mode_t st_mode;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
} stat_t;

#define MAXDEF_FILENO 3

extern file_t sysfile[MAXDEF_FILENO];

#include <bits/access.h>
#include <bits/fcntl.h>
#include <bits/fileno.h>
#include <bits/lseek.h>

unsigned dir_get_type(mode_t mode);
bool dir_emit(dirctx_t *ctx, const char *name, size_t len, u64 ino, unsigned type);
bool dir_emit_node(dirctx_t *ctx, node_t *chd);
bool dir_emit_dot(dirctx_t *ctx);
bool dir_emit_dotdot(dirctx_t *ctx);

#include <textos/ioctl.h>

#endif
