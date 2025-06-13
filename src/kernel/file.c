#include <textos/task.h>
#include <textos/file.h>
#include <textos/errno.h>
#include <textos/assert.h>
#include <textos/fs/pipe.h>
#include <textos/syscall.h>

#include <textos/mm.h>

int fd_get()
{
    file_t **ft = task_current()->files;
    for (int i = 0 ; i < MAX_FILE ; i++)
    {
        if (!ft[i])
            return i;
    }
    return -EMFILE;
}

int file_get(int *new, file_t **file)
{
    file_t **ft = task_current()->files;
    int fd = fd_get();
    if (!(ft[fd] = malloc(sizeof(file_t))))
        return -ENOMEM;

    *file = ft[fd];
    return *new = fd;
}

// device number

static uint get_major(long x)
{
    return (uint)(((x >> 32) & 0xfffff000) | ((x >> 8) & 0xfff));
}

static uint get_minor(long x)
{
    return (uint)(((x >> 12) & 0xffffff00) | (x & 0xff));
}

static long make_dev(uint x, uint y)
{
    return (long)
       ((x & 0xfffff000ULL) << 32) | ((x & 0xfffULL) << 8) |
       ((y & 0xffffff00ULL) << 12) | ((y & 0xffULL));
}

// TODO: flgs
static inline u64 parse_args(int x)
{
    int v = 0;
    if (x & O_CREAT)
        v |= VFS_CREATE;
    if (x & O_DIRECTORY)
        v |= VFS_DIR;

    return v;
}

__SYSCALL_DEFINE2(int, open, char *, path, int, flgs)
{
    node_t *node;
    file_t *file;
    u64 opargs = parse_args(flgs);
    
    int ret;
    if ((ret = vfs_open(task_current()->pwd, &node, path, opargs)) < 0)
        return ret;

    int fd;
    if (file_get(&fd, &file) < 0)
        return -EMFILE;

    dirctx_t *dirctx = NULL;
    if (flgs & O_DIRECTORY)
    {
        dirctx = malloc(sizeof(dirctx_t));
        dirctx->stat = ctx_inv;
    }

    file->refer = 1;
    file->offset = 0;
    file->node = node;
    file->dirctx = dirctx;
    file->flgs = flgs;

fail:
    return fd;
}

// todo: max size limited
__SYSCALL_DEFINE3(ssize_t, write, int, fd, void *, buf, size_t, cnt)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    int accm = file->flgs & O_ACCMODE;
    if (accm == O_RDONLY)
        return -EBADF;

    int ret = file->node->opts->write(file->node, buf, cnt, file->offset);
    if (ret < 0)
        return ret;
    
    file->offset += ret;
    return ret;
}

#include <string.h>

__SYSCALL_DEFINE3(ssize_t, readdir, int, fd, void *, buf, size_t, mx)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;
        
    int accm = file->flgs & O_ACCMODE;
    if (accm == O_WRONLY)
        return -EBADF;

    dirctx_t *ctx = file->dirctx;
    ctx->buf = buf;
    ctx->bufmx = mx;
    ctx->bufused = 0;
    ctx->bufents = 0;

    if (file->node->opts->readdir(file->node, ctx) < 0)
        return EOF;

    file->offset = ctx->pos;
    return ctx->bufused;
}

/**
 * @param ino    unused yet
 * @param type   unused yet
 */
bool __dir_emit(dirctx_t *ctx, const char *name, size_t len, u64 ino, unsigned type)
{
    size_t siz = sizeof(dir_t) + len + 1;
    if (ctx->bufused + siz > ctx->bufmx)
        return false;

    dir_t *dir = ctx->buf;
    dir->idx = ctx->pos;
    dir->siz = siz;
    dir->len = len;
    strcpy(dir->name, name);

    ctx->bufused += siz;
    ctx->buf += siz;
    return ctx->bufused != ctx->bufmx;
}

bool __dir_emit_node(dirctx_t *ctx, node_t *chd)
{
    return __dir_emit(ctx, chd->name, strlen(chd->name), 0, 0);
}

__SYSCALL_DEFINE2(int, seekdir, int, fd, size_t *, pos)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    return file->node->opts->seekdir(file->node, file->dirctx, pos);
}

/*
 * NOTE: actually the dev file uses the op of the
 *       filesystem in which it locates.
 */
__SYSCALL_DEFINE3(ssize_t, read, int, fd, void *, buf, size_t, cnt)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    if (file->flgs & O_DIRECTORY)
        return readdir(fd, buf, cnt);

    int accm = file->flgs & O_ACCMODE;
    if (accm == O_WRONLY)
        return -EBADF;

    int ret = file->node->opts->read(file->node, buf, cnt, file->offset);
    if (ret < 0)
        return ret;
    
    file->offset += ret;
    return ret;
}

__SYSCALL_DEFINE1(int, close, int, fd)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    if (--file->refer > 0) {
        task_current()->files[fd] = NULL;
        return 0;
    }

    int ret = file->node->opts->close(file->node);
    
    free(file);
    task_current()->files[fd] = NULL;
    return ret;
}

__SYSCALL_DEFINE2(int, stat, char *, path, stat_t *, sb) 
{
    int ret;
    node_t *node;
    
    ret = vfs_open(task_current()->pwd, &node, path, VFS_GAIN);
    if (ret < 0)
        return ret;

    int mode = 0;
    if (node->attr & NA_DIR)
        mode |= S_IFDIR;

    devst_t *d;
    if (node->attr & NA_DEV)
        d = node->pdata;
    else
        d = *(devst_t **)node->sys;

    sb->siz = node->siz;
    sb->dev = make_dev(d->major, d->minor);
    sb->mode = mode;

    return 0;
}

__SYSCALL_DEFINE3(int, ioctl, int, fd, int, req, void *, argp)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;
    
    node_t *node = file->node;
    if (node->attr & NA_DEV)
    {
        devst_t *dev = node->pdata;
        return dev->ioctl(dev, req, argp);
    }

    return node->opts->ioctl(node, req, argp);
}

__SYSCALL_DEFINE2(int, dup2, int, old, int, new)
{
    file_t **ft = task_current()->files;
    file_t *file = ft[old];
    if (!file)
        return -EBADF;

    if (old == new)
        return -EINVAL;

    // if newfd exists, close it first
    if (ft[new])
        close(new);

    file->refer++;
    ft[new] = file;
    return new;
}

__SYSCALL_DEFINE1(int, dup, int, fd)
{
    int new = fd_get();
    if (new < 0)
        return -EMFILE;

    return dup2(fd, new);
}

__SYSCALL_DEFINE1(int, pipe, int *, fds)
{
    int fd0, fd1;
    file_t *f0, *f1;
    ASSERTK(file_get(&fd0, &f0) >= 0);
    ASSERTK(file_get(&fd1, &f1) >= 0);

    node_t *n = malloc(sizeof(*n));
    ASSERTK(n != NULL);

    pipe_init(n);

    f0->node = n;
    f1->refer = 1;
    f0->spec = S_PIPE_R;
    f0->flgs = O_RDONLY;

    f1->node = n;
    f0->refer = 1;
    f1->flgs = O_WRONLY;
    f1->spec = S_PIPE_W;

    fds[0] = fd0;
    fds[1] = fd1;
    return 0;
}

__SYSCALL_DEFINE3(int, mknod, char *, path, int, mode, long, dev)
{
    uint major = get_major(dev);
    uint minor = get_minor(dev);
    devst_t *d = dev_lookup_nr(major, minor);
    return vfs_mknod(path, d);
}

__SYSCALL_DEFINE2(int, mount, char *, src, char *, dst)
{
    int ret;
    node_t *sn, *dn;

    ret = vfs_open(task_current()->pwd, &sn, src, 0);
    if (ret < 0)
        return ret;

    if (!(sn->attr & NA_DEV))
        return -ENOBLK;

    devst_t *dev = sn->pdata;
    if (dev->type != DEV_BLK)
        return -ENOBLK;

    if (dev->subtype != DEV_PART)
        return -EINVAL;

    ret = vfs_open(task_current()->pwd, &dn, dst, VFS_DIR);
    if (ret < 0)
        return ret;

    node_t *root = extract_part(dev);
    return vfs_mount(dn, root);
}

__SYSCALL_DEFINE2(int, umount2, char *, target, int, flags)
{
    int ret;
    node_t *dn;

    ret = vfs_open(task_current()->pwd, &dn, target, VFS_GAIN | VFS_GAINMNT);
    if (ret < 0)
        return ret;
    
    ret = vfs_umount(dn);
    return ret;
}

__SYSCALL_DEFINE1(int, chdir, char *, path)
{
    int ret;
    node_t *node;
    task_t *task = task_current();

    ret = vfs_open(task->pwd, &node, path, VFS_DIR);
    if (ret < 0)
        return ret;

    task->pwd = node;
    return ret;
}

__SYSCALL_DEFINE2(int, mkdir, char *, path, int, mode)
{
    int ret;
    node_t *node;
    task_t *task = task_current();

    ret = vfs_open(task->pwd, &node, path, VFS_DIR);
    if (ret >= 0)
    {
        return -EEXIST;
    }

    ret = vfs_open(task->pwd, &node, path, VFS_DIR | VFS_CREATE);
    return ret;
}

__SYSCALL_DEFINE1(int, rmdir, char *, path)
{
    return -EPERM;
}
