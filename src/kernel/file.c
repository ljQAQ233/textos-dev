#include <textos/task.h>
#include <textos/file.h>
#include <textos/errno.h>
#include <textos/assert.h>
#include <textos/fs/pipe.h>

#include <textos/mm.h>

int fd_get()
{
    file_t **ft = task_current()->files;
    for (int i = 0 ; i < MAX_FILE ; i++)
    {
        if (!ft[i])
            return i;
    }
    return -1;
}

int file_get(int *new, file_t **file)
{
    file_t **ft = task_current()->files;
    int fd = fd_get();
    if (!(ft[fd] = malloc(sizeof(file_t))))
        return -1;

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

int open(char *path, int flgs)
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
ssize_t write(int fd, void *buf, size_t cnt)
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

ssize_t readdir(int fd, void *buf, size_t mx)
{
    file_t *file = task_current()->files[fd];
    if (!file)
        return -EBADF;
        
    int accm = file->flgs & O_ACCMODE;
    if (accm == O_WRONLY)
        return -EBADF;

    size_t cnt = 0;
    while (true) {
        node_t *chd;
        if (file->node->opts->readdir(file->node, &chd, file->dirctx) < 0)
            break;

        size_t len = strlen(chd->name);
        size_t siz = sizeof(dir_t) + len + 1;
        if (cnt + siz > mx)
            break;

        dir_t *dir = buf;
        dir->idx = file->offset;
        dir->siz = siz;
        dir->len = len;
        strcpy(dir->name, chd->name);

        cnt += siz;
        buf += siz;
        file->offset += 1;
    }
    return cnt;
}

ssize_t read(int fd, void *buf, size_t cnt)
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
        return -1;
    
    file->offset += ret;
    return ret;
}

int close(int fd)
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

int stat(char *path, stat_t *sb)
{
    int ret;
    node_t *node;
    
    ret = vfs_open(task_current()->pwd, &node, path, VFS_GAIN);
    if (ret < 0)
        return ret;

    int mode = 0;
    if (node->attr & NA_DIR)
        mode |= S_IFDIR;

    dev_t *d;
    if (node->attr & NA_DEV)
        d = node->pdata;
    else
        d = *(dev_t **)node->sys;

    sb->siz = node->siz;
    sb->dev = make_dev(d->major, d->minor);
    sb->mode = mode;

    return 0;
}

int dup2(int old, int new)
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

int dup(int fd)
{
    int new = fd_get();
    if (new < 0)
        return -EMFILE;

    return dup2(fd, new);
}

int pipe(int fds[2])
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

int mknod(char *path, int mode, long dev)
{
    uint major = get_major(dev);
    uint minor = get_minor(dev);
    dev_t *d = dev_lookup_nr(major, minor);
    return vfs_mknod(path, d);
}

int mount(char *src, char *dst)
{
    int ret;
    node_t *sn, *dn;

    ret = vfs_open(task_current()->pwd, &sn, src, 0);
    if (ret < 0)
        return ret;

    if (!(sn->attr & NA_DEV))
        return -ENOBLK;

    dev_t *dev = sn->pdata;
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

int chdir(char *path)
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

int mkdir(char *path, int mode)
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

int rmdir(char *path)
{
    return -EPERM;
}
