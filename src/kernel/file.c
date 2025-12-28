#include <textos/task.h>
#include <textos/file.h>
#include <textos/errno.h>
#include <textos/assert.h>
#include <textos/fs/pipe.h>
#include <textos/syscall.h>

#include <textos/mm.h>

int fd_get(int min)
{
    file_t **ft = task_current()->files;
    for (int i = min ; i < MAX_FILE ; i++)
    {
        if (!ft[i])
            return i;
    }
    return -EMFILE;
}

int file_get(int *new, file_t **file, int min)
{
    file_t **ft = task_current()->files;
    int fd = fd_get(min);
    if (!(ft[fd] = malloc(sizeof(file_t))))
        return -ENOMEM;

    *file = ft[fd];
    return *new = fd;
}

int file_put(int fd)
{
    file_t **ft = task_current()->files;
    if (!ft[fd])
        return -EMFILE;
    free(ft[fd]);
    ft[fd] = NULL;
    return 0;
}

__SYSCALL_DEFINE3(int, open, char *, path, int, flgs, int, mode)
{
    node_t *node;
    file_t *file;
    size_t off = 0;
    mode &= 0777;
    
    int ret;
    if ((ret = vfs_open(task_current()->pwd, path, flgs, mode, &node)) < 0)
        return ret;

    int fd;
    if (file_get(&fd, &file, 0) < 0)
    {
        ret = -EMFILE;
        goto fail;
    }
    
    if (flgs & (O_WRONLY | O_RDWR))
    {
        if (flgs & O_TRUNC)
            vfs_truncate(node, 0);
    }
    else if (flgs & O_APPEND)
    {
        ret = -EINVAL;
        goto fail;
    }

    dirctx_t *dirctx = NULL;
    if (flgs & O_DIRECTORY)
    {
        dirctx = malloc(sizeof(dirctx_t));
        dirctx->stat = ctx_inv;
    }

    file->refer = 1;
    file->offset = off;
    file->node = node;
    file->dirctx = dirctx;
    file->flgs = flgs;
    return fd;

fail:
    if (fd >= 0)
        file_put(fd);
    return ret;
}

int dup2(int old, int new);

__SYSCALL_DEFINE3(int, fcntl, int, fd, int, cmd, long, arg)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    switch (cmd)
    {
    case F_DUPFD:
        {
            int newfd;
            int ret = file_get(&newfd, &file, arg);
            if (ret < 0)
                return fd;
            return dup2(fd, newfd);
        }
    case F_GETFD:
        {
            return file->fdfl;
        }
    case F_SETFD:
        {
            int fdfl = (int)arg;
            file->fdfl = fdfl;
            return 0;
        }
    case F_GETFL:
        {
            return file->flgs;
        }
    case F_SETFL:
        {
            int flgs = (int)arg;
            int mask = O_APPEND | 0;
            file->flgs &= ~mask;
            file->flgs |= flgs;
            return 0;
        }
    case F_GETLK:
    case F_SETLK:
    case F_SETLKW:
    case F_SETOWN:
    case F_GETOWN:
    case F_SETSIG:
    case F_GETSIG:
        return -ENOSYS;
    }
    return -EINVAL;
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
        return -EISDIR;

    int accm = file->flgs & O_ACCMODE;
    if (accm == O_WRONLY)
        return -EBADF;

    int ret = file->node->opts->read(file->node, buf, cnt, file->offset);
    if (ret < 0)
        return ret;
    
    file->offset += ret;
    return ret;
}

__SYSCALL_DEFINE3(ssize_t, readv, int, fd, const iovec_t *, iov, int, iovcnt)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    if (file->flgs & O_DIRECTORY)
        return -EISDIR;
    
    if (!iov || iovcnt < 0 || iovcnt >= IOV_MAX)
        return -EINVAL;

    int accm = file->flgs & O_ACCMODE;
    if (accm == O_WRONLY)
        return -EBADF;

    size_t oldoff = file->offset;
    ssize_t sum = 0;
    ssize_t ret = 0;
    for (int i = 0 ; i < iovcnt ; i++)
    {
        if (iov[i].iov_len == 0)
            continue;
        if (iov[i].iov_base == NULL)
        {
            ret = -EINVAL;
            goto rollback;
        }
        ret = file->node->opts->read(file->node, iov[i].iov_base, iov[i].iov_len, file->offset);
        if (ret < 0)
            goto rollback;
        if (ret == 0)
            break;
        sum += ret;
    }
    
    file->offset += sum;
    return sum;

rollback:
    file->offset = oldoff;
    return ret;
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

    size_t off;
    if (file->flgs & O_APPEND)
        off = file->node->siz;
    else
        off = file->offset;

    int ret = file->node->opts->write(file->node, buf, cnt, off);
    if (ret < 0)
        return ret;
    
    file->offset = off + ret;
    return ret;
}

__SYSCALL_DEFINE3(ssize_t, writev, int, fd, const iovec_t *, iov, int, iovcnt)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    if (file->flgs & O_DIRECTORY)
        return -EISDIR;
    
    if (!iov || iovcnt < 0 || iovcnt >= IOV_MAX)
        return -EINVAL;

    int accm = file->flgs & O_ACCMODE;
    if (accm == O_WRONLY)
        return -EBADF;

    size_t oldoff = file->offset;
    ssize_t sum = 0;
    int ret = 0;
    for (int i = 0 ; i < iovcnt ; i++)
    {
        if (iov[i].iov_len == 0)
            continue;
        if (iov[i].iov_base == NULL)
        {
            ret = -EINVAL;
            goto rollback;
        }
        ret = file->node->opts->write(file->node, iov[i].iov_base, iov[i].iov_len, file->offset);
        if (ret < 0)
        {
            if (ret != -ENOSPC)
                goto rollback;
            ret = 0;
        }
        if (ret == 0)
            break;
        sum += ret;
    }
    
    file->offset += sum;
    return sum;

rollback:
    file->offset = oldoff;
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
bool dir_emit(dirctx_t *ctx, const char *name, size_t len, u64 ino, unsigned type)
{
    size_t siz = sizeof(dir_t) + len + 1;
    if (ctx->bufused + siz > ctx->bufmx)
        return false;

    dir_t *dir = ctx->buf;
    dir->idx = ctx->pos;
    dir->type = type;
    dir->ino = ino;
    dir->siz = siz;
    dir->len = len;
    strcpy(dir->name, name);

    ctx->bufused += siz;
    ctx->buf += siz;
    return true;
}

unsigned dir_get_type(mode_t mode)
{
    if (S_ISREG(mode))
        return DT_REG;
    else if (S_ISDIR(mode))
        return DT_DIR;
    else if (S_ISLNK(mode))
        return DT_LNK;
    else if (S_ISCHR(mode))
        return DT_CHR;
    else if (S_ISBLK(mode))
        return DT_BLK;
    else if (S_ISFIFO(mode))
        return DT_FIFO;
    else if (S_ISSOCK(mode))
        return DT_SOCK;
    else
        return DT_UNKNOWN;
}

bool dir_emit_node(dirctx_t *ctx, node_t *n)
{
    int type = dir_get_type(n->mode);
    return dir_emit(ctx, n->name, strlen(n->name), n->ino, type);
}

bool dir_emit_dot(dirctx_t *ctx)
{
    return dir_emit(ctx, ".", 1, ctx->node->ino, dir_get_type(ctx->node->mode));
}

bool dir_emit_dotdot(dirctx_t *ctx)
{
    node_t *prt = vfs_getprt(ctx->node);
    return dir_emit(ctx, "..", 2, prt->ino, dir_get_type(prt->mode));
}

__SYSCALL_DEFINE2(int, seekdir, int, fd, size_t *, pos)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    return file->node->opts->seekdir(file->node, file->dirctx, pos);
}

__SYSCALL_DEFINE3(off_t, lseek, int, fd, off_t, off, int, whence)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;
    if (S_ISDIR(file->node->mode))
        return -EISDIR;
    switch (whence)
    {
    case SEEK_SET:
        file->offset = off;
        break;
    case SEEK_CUR:
        file->offset += off;
        break;
    case SEEK_END:
        file->offset = file->node->siz + off;
        break;
    }
    return file->offset;
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
    DEBUGK(K_TRACE, "task[#%d] close %d\n", task_current()->pid, fd);
    return ret;
}

static void fillsb(node_t *node, stat_t *sb)
{
    sb->st_dev = node->dev;
    sb->st_ino = node->ino;
    sb->st_nlink = 1;
    sb->st_mode = node->mode;
    sb->st_uid = node->uid;
    sb->st_gid = node->gid;
    sb->st_rdev = node->rdev;
    sb->st_size = node->siz;
    sb->st_blksize = -1; // TODO
    sb->st_blocks = -1;
    sb->st_atime = node->atime;
    sb->st_mtime = node->mtime;
    sb->st_ctime = node->ctime;
}

__SYSCALL_DEFINE2(int, stat, char *, path, stat_t *, sb) 
{
    int ret;
    node_t *node;
    
    ret = vfs_open(task_current()->pwd, path, FS_GAIN, 0, &node);
    if (ret < 0)
        return ret;
    fillsb(node, sb);
    return 0;
}

__SYSCALL_DEFINE2(int, fstat, int, fd, stat_t *, sb)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;
    fillsb(file->node, sb);
    return 0;
}

__SYSCALL_DEFINE2(int, access, const char *, path, int, amode)
{
    int ret;
    node_t *node;
    
    ret = vfs_open(task_current()->pwd, path, FS_GAIN, 0, &node);
    if (ret < 0)
        return ret;
    if (amode == F_OK)
        return 0;
    
    int want = 0;
    if (amode & X_OK) want |= MAY_EXEC;
    if (amode & W_OK) want |= MAY_WRITE;
    if (amode & R_OK) want |= MAY_READ;
    return vfs_permission(node, want);
}

__SYSCALL_DEFINE3(int, ioctl, int, fd, int, req, void *, argp)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;
    
    node_t *node = file->node;
    if (S_ISCHR(node->mode)
     || S_ISBLK(node->mode)
     || S_ISSOCK(node->mode))
    {
        uint ma = major(node->rdev);
        uint mi = minor(node->rdev);
        devst_t *dev = dev_lookup_nr(ma, mi);
        if (!dev)
            return -ENODEV;
        return dev->ioctl(dev, req, argp);
    }
    return -EINVAL;
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
    int new = fd_get(0);
    if (new < 0)
        return -EMFILE;

    return dup2(fd, new);
}

__SYSCALL_DEFINE1(int, pipe, int *, fds)
{
    int fd0, fd1;
    file_t *f0, *f1;
    ASSERTK(file_get(&fd0, &f0, 0) >= 0);
    ASSERTK(file_get(&fd1, &f1, 0) >= 0);

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
    if (!S_ISCHR(mode)
     && !S_ISBLK(mode)
     && !S_ISFIFO(mode))
        return -EINVAL;
    return vfs_mknod(path, dev, mode);
}

__SYSCALL_DEFINE3(int, chown, char *, path, uid_t, owner, gid_t, group)
{
    int ret;
    node_t *node;

    ret = vfs_open(task_current()->pwd, path, 0, 0, &node);
    if (ret < 0)
        return ret;
    
    ret = vfs_chown(node, owner, group);
    if (ret < 0)
        return ret;
    return ret;
}

__SYSCALL_DEFINE3(int, fchown, int, fd, uid_t, owner, gid_t, group)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    int ret = vfs_chown(file->node, owner, group);
    if (ret < 0)
        return ret;
    return ret;
}

__SYSCALL_DEFINE2(int, chmod, char *, path, mode_t, mode)
{
    int ret;
    node_t *node;

    ret = vfs_open(task_current()->pwd, path, 0, 0, &node);
    if (ret < 0)
        return ret;
    
    mode &= 07777;
    ret = vfs_chmod(node, mode);
    if (ret < 0)
        return ret;
    return ret;
}

__SYSCALL_DEFINE2(int, fchmod, int, fd, mode_t, mode)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;

    mode &= 07777;
    int ret = vfs_chmod(file->node, mode);
    if (ret < 0)
        return ret;
    return ret;
}

__SYSCALL_DEFINE2(int, mount, char *, src, char *, dst)
{
    int ret;
    node_t *sn, *dn;

    ret = vfs_open(task_current()->pwd, src, 0, 0, &sn);
    if (ret < 0)
        return ret;

    if (!S_ISBLK(sn->mode))
        return -ENOBLK;

    uint ma = major(sn->rdev);
    uint mi = minor(sn->rdev);
    devst_t *dev = dev_lookup_nr(ma, mi);
    if (!dev)
        return -ENOBLK;
    if (dev->type != DEV_BLK)
        return -ENOBLK;

    if (dev->subtype != DEV_PART)
        return -EINVAL;

    ret = vfs_open(task_current()->pwd, dst, O_DIRECTORY, 0, &dn);
    if (ret < 0)
        return ret;

    node_t *root = extract_part(dev);
    return vfs_mount(dn, root);
}

__SYSCALL_DEFINE2(int, umount2, char *, target, int, flags)
{
    int ret;
    node_t *dn;

    ret = vfs_open(task_current()->pwd, target, FS_GAIN | FS_GAINMNT, 0, &dn);
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

    ret = vfs_open(task->pwd, path, O_DIRECTORY, 0, &node);
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
    mode &= 0777;

    ret = vfs_open(task->pwd, path, O_DIRECTORY, 0, &node);
    if (ret >= 0)
    {
        return -EEXIST;
    }

    ret = vfs_open(task->pwd, path, O_DIRECTORY | O_CREAT, mode, &node);
    return ret;
}

__SYSCALL_DEFINE1(int, rmdir, char *, path)
{
    return -EPERM;
}

__SYSCALL_DEFINE2(char *, getcwd, char *, buf, size_t, size)
{
    size_t sz = size;
    int ret = vfs_getpath(task_current()->pwd, buf, &sz);
    if (ret < 0)
        return (char *)(intptr_t)ret;
    return buf;
}
