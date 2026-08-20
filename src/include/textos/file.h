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

#define __NEED_dir_t
#include <bits/dirent.h>

#include <bits/mode.h>
#include <bits/perm.h>
#include <bits/stat.h>

#include <bits/access.h>
#include <bits/fcntl.h>
#include <bits/fileno.h>
#include <bits/lseek.h>

unsigned dir_get_type(mode_t mode);
bool dir_emit(dirctx_t *ctx, const char *name, size_t len, u64 ino,
              unsigned type);
bool dir_emit_node(dirctx_t *ctx, node_t *chd);
bool dir_emit_dot(dirctx_t *ctx);
bool dir_emit_dotdot(dirctx_t *ctx);

#include <textos/ioctl.h>

#endif
