#include <textos/task.h>
#include <textos/file.h>
#include <textos/errno.h>

#define NODE(x, label)        \
    [x] = {                   \
        .name = label,        \
        .opts =               \
            &(fs_opts_t) { }, \
    }                         \

static node_t file[3] = {
    NODE(STDIN_FILENO,  "stdin"),
    NODE(STDOUT_FILENO, "stdout"),
    NODE(STDERR_FILENO, "stderr"),
};

#define FILE(x)            \
    [x] = {                \
        .occupied = 1,     \
        .offset = 0,       \
        .node = &file[x],  \
        .flgs = O_ACCMODE, \
    }                      \

file_t sysfile[MAXDEF_FILENO] = {
    FILE(STDIN_FILENO),
    FILE(STDOUT_FILENO),
    FILE(STDERR_FILENO),
};

static int get_free(int *fd, file_t **file)
{
    *fd = -1;
    file_t *p = task_current()->files;
    for (int i = 0 ; i < MAX_FILE ; i++, p++)
        if (!p->occupied)
        {
            p->occupied = true;
            *fd = i;
            *file = p;
            break;
        }

    return *fd;
}

// TODO: flgs
static inline u64 parse_args(int *flgs)
{
    int x = *flgs, v = 0;
    if (x & O_CREAT)
        v |= O_CREATE;

    return v;
}

int open(char *path, int flgs)
{
    int fd;
    int errno = 0;
    u64 opargs = parse_args(&flgs);
    node_t *node;
    file_t *file;
    if ((errno = vfs_open(task_current()->pwd, &node, path, opargs)) < 0)
        goto fail;

    if (get_free(&fd, &file) < 0)
    {
        errno = -EMFILE;
        goto fail;
    }
    file->node = node;
    file->flgs = flgs;

fail:
    return fd;
}

// todo: max size limited
ssize_t write(int fd, void *buf, size_t cnt)
{
    int errno = 0;
    file_t *file = &task_current()->files[fd];

    int accm = file->flgs & O_ACCMODE;
    if (accm == O_RDONLY) {
        errno = -EBADF;
        return -1;
    }
    int res = file->node->opts->write(file->node, buf, cnt, file->offset);
    if (res < 0)
        return -1;
    
    file->offset += cnt;
    return cnt;
}

ssize_t read(int fd, void *buf, size_t cnt)
{
    int errno = 0;
    file_t *file = &task_current()->files[fd];

    int accm = file->flgs & O_ACCMODE;
    if (accm != 0 && accm != O_ACCMODE) {
        errno = -EBADF;
        return -1;
    }

    int res = file->node->opts->read(file->node, buf, cnt, file->offset);
    if (res < 0)
        return -1;
    
    file->offset += cnt;
    return cnt;
}

int close(int fd)
{
    file_t *file = &task_current()->files[fd];
    int res = file->node->opts->close(file->node);
    if (res < 0)
        return -1;
    
    return 0;
}

#include <textos/dev.h>

// system stdio

int kncon_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    dev_t *con = dev_lookup_type(DEV_CHAR, DEV_KNCON);
    return con->write(con, buf, siz);
}

int kncon_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    dev_t *con = dev_lookup_type(DEV_CHAR, DEV_KNCON);
    return con->read(con, buf, siz);
}

int kncon_exit(node_t *this)
{
    return 0;
}

void fd_init()
{
    for (int i = 0 ; i < MAXDEF_FILENO ; i++)
    {
        vfs_initops(file[i].opts);
        file[i].opts->close = kncon_exit;
    }

    file[STDOUT_FILENO].opts->write = kncon_write;
    file[STDERR_FILENO].opts->write = kncon_write;
    file[STDIN_FILENO].opts->read = kncon_read;
}
