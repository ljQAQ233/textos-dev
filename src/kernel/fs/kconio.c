#include <textos/fs.h>
#include <textos/dev.h>
#include <textos/file.h>

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
        .offset = 0,       \
        .node = &file[x],  \
        .flgs = O_ACCMODE, \
        .refer = 255,      \
    }                      \

file_t sysfile[MAXDEF_FILENO] = {
    FILE(STDIN_FILENO),
    FILE(STDOUT_FILENO),
    FILE(STDERR_FILENO),
};

// system stdio

static int kncon_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    dev_t *con = dev_lookup_type(DEV_KNCON, 0);
    return con->write(con, buf, siz);
}

static int kncon_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    dev_t *con = dev_lookup_type(DEV_KNCON, 0);
    return con->read(con, buf, siz);
}

static int kncon_exit(node_t *this)
{
    return 0;
}

void __kconio_init()
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