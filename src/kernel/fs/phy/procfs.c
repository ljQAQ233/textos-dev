
/*
 */

#include <textos/file.h>
#include <textos/fs.h>
#include <textos/fs/inter.h>
#include <textos/panic.h>
#include <textos/mm/heap.h>
#include <textos/klib/string.h>
#include <textos/args.h>
#include <textos/klib/vsprintf.h>

struct proc_opts;
struct proc_entry;

/*
 * for a regular file, any of op may be nullptr, but for a dir registered by
 * `proc_mkdir`, op would be either `opxxx` or `noopt`.
 */
typedef struct proc_opts
{
    int  (*open)(struct proc_entry *parent, char *name, struct proc_entry **res);
    int  (*ioctl)(struct proc_entry *this, int req, void *argp);
    int  (*close)(struct proc_entry *this);
    int  (*read)(struct proc_entry *this, void *buf, size_t siz, size_t offset);
    int  (*write)(struct proc_entry *this, void *buf, size_t siz, size_t offset);
    int  (*truncate)(struct proc_entry *this, size_t offset);
    int  (*readdir)(struct proc_entry *this, dirctx_t *ctx, int relative_pos);
    int  (*seekdir)(struct proc_entry *this, dirctx_t *ctx, size_t *relative_pos);
    void *(*mmap)(struct proc_entry *this, vm_region_t *vm);
} proc_opts_t;

typedef struct proc_entry
{
    const char *name;
    u64 ino;
    int mode;
    bool is_dir;
    const proc_opts_t *op;
    struct proc_entry *parent;
    struct proc_entry *subdir;
    struct proc_entry *next;
    void *pdata;
} proc_entry_t;

typedef struct proc_super
{
    dev_t *dev;
} proc_super_t;

#define goto_if(cond, label) do { if (cond) goto label; } while (0)

bool procfs_dir_emit(dirctx_t *ctx, const char *name, size_t len, u64 ino, unsigned type)
{
    if (__dir_emit(ctx, name, len, ino, type))
    {
        ctx->pos++;
        return true;
    }
    return false;
}

static bool procfs_dir_emit_ent(dirctx_t *ctx, proc_entry_t *ent)
{
    unsigned type = ent->is_dir ? DT_DIR : DT_REG;
    if (__dir_emit(ctx, ent->name, strlen(ent->name), 0, type))
    {
        ctx->pos++;
        return true;
    }
    return false;
}

u64 __procfs_ino = 1;
proc_super_t __procfs_super;
proc_entry_t __procfs_root;

#define proc_foreach(ent) \
    for (proc_entry_t *ptr = ent ; ptr ; ptr = ptr->next)

proc_entry_t *proc_ent_find(proc_entry_t *dir, char *name)
{
    proc_foreach(dir->subdir) {
        if (strcmp(ptr->name, name) == 0)
            return ptr;
    }
    return NULL;
}

int proc_ent_count(proc_entry_t *dir)
{
    int count = 0;
    proc_foreach(dir->subdir) {
        count++;
    }
    return count;
}

static int proc_name_chk(char *name)
{
    // TODO
    return 0;
}

static u64 proc_ino_alloc()
{
    return __procfs_ino++;
}

static void proc_ent_reg(proc_entry_t *prt, proc_entry_t *chd)
{
    chd->parent = prt;
    chd->next = prt->subdir;
    prt->subdir = chd;
}

/**
 * @brief create a file entry in `dir`
 * 
 * @param name file name
 * @param mode unix mode
 * @param prt  parent dir. NULL refers to procfs' root
 * @param op   procfs operation
 * @return proc_entry_t *
 */
proc_entry_t *proc_create(char *name, int mode, proc_entry_t *prt, proc_opts_t *op)
{
    if (!prt)
        prt = &__procfs_root;
    if (proc_name_chk(name) < 0)
        return NULL;
    if (proc_ent_find(prt, name))
        return NULL;
    
    proc_entry_t *ent = malloc(sizeof(proc_entry_t));
    if (!ent)
        return NULL;

    ent->name = name;
    ent->ino = proc_ino_alloc();
    ent->mode = mode;
    ent->op = op;
    proc_ent_reg(prt, ent);
    return ent;
}

/**
 * @brief create a directory entry in `prt`, 
 * 
 * @param name dir name
 * @param prt  parent
 * @param op   merely `readdir` / `seekdir` are valid
 * @return proc_entry_t* 
 */
proc_entry_t *proc_mkdir(char *name, proc_entry_t *prt, proc_opts_t *op)
{
    if (!prt)
        prt = &__procfs_root;
    if (proc_name_chk(name) < 0)
        return NULL;
    if (proc_ent_find(prt, name))
        return NULL;
    
    proc_entry_t *dir = malloc(sizeof(proc_entry_t));
    if (!dir)
        return NULL;

    op->open = noopt;
    op->ioctl = noopt;
    op->close = noopt;
    op->read = noopt;
    op->write = noopt;
    op->truncate = noopt;
    // func readdir here;
    // func seekdir here;
    op->mmap = noopt;

    dir->name = name;
    dir->ino = proc_ino_alloc();
    dir->mode = 0555;
    dir->is_dir = true;
    dir->op = op;
    proc_ent_reg(prt, dir);
    return dir;
}

void proc_remove(proc_entry_t *ent)
{
    if (ent == &__procfs_root)
        PANIC("not allowed to remove procfs root!\n");

    while (ent->subdir)
    {
        proc_entry_t *old = ent->subdir;
        ent->subdir = old->next;
        proc_remove(old);
    }

    proc_entry_t **pp = &ent->parent->subdir;
    while (*pp && *pp != ent)
        pp = &(*pp)->next;
    if (*pp == ent)
        *pp = ent->next;
    free(ent);
}

fs_opts_t __procfs_op;

static node_t *procfs_nodeget(proc_entry_t *ent)
{
    node_t *node = malloc(sizeof(node_t));
    node->name = strdup(ent->name);
    node->attr = ent->is_dir ? NA_DIR : NA_REG;
    node->siz = 0;
    node->atime = arch_time_now();
    node->mtime = arch_time_now();
    node->ctime = arch_time_now();
    node->root = NULL;
    node->parent = NULL;
    node->child = NULL;
    node->next = NULL;
    node->sys = &__procfs_super;
    node->systype = 0;
    node->idx = 0;
    node->pdata = ent;
    node->mount = NULL;
    node->opts = &__procfs_op;
    return node;
}

#include <textos/errno.h>

static int procfs_open(node_t *parent, char *name, u64 args, node_t **result)
{
    proc_entry_t *dir = parent->pdata;
    proc_entry_t *chd = proc_ent_find(dir, name);

    if (!chd) {
        int ret = dir->op->open(dir, name, &chd);
        if (ret < 0)
            return ret;
    }

    node_t *node = procfs_nodeget(chd);
    node->parent = parent;
    node->next = parent->next;
    parent->next = node;
    *result = node;
    return 0;
}

static int procfs_ioctl(node_t *this, int req, void *argp)
{
    proc_entry_t *ent = this->pdata;
    if (ent->op->ioctl)
        return ent->op->ioctl(ent, req, argp);
    return -EINVAL;
}

static int procfs_close(node_t *this)
{
    proc_entry_t *ent = this->pdata;
    if (ent->op->close) {
        int ret = ent->op->close(ent);
        if (ret < 0)
            return ret;
    }
    return vfs_release(this);
}

static int procfs_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    proc_entry_t *ent = this->pdata;
    if (ent->op->read)
        return ent->op->read(ent, buf, siz, offset);
    return -EINVAL;
}

int procfs_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    proc_entry_t *ent = this->pdata;
    if (ent->op->write)
        return ent->op->write(ent, buf, siz, offset);
    return -EINVAL;
}

static int procfs_readdir(node_t *node, dirctx_t *ctx)
{
    proc_entry_t *dir = node->pdata;
    // .
    if (ctx->pos == 0) {
        goto_if(!procfs_dir_emit(ctx, ".", 1, dir->ino, S_IFDIR), end);
    }
    // ..
    if (ctx->pos == 1)
    {
        if (dir == &__procfs_root)
            goto_if(!procfs_dir_emit(ctx, "..", 2, node->parent->idx, S_IFDIR), end);
        else
            goto_if(!procfs_dir_emit(ctx, "..", 2, dir->parent->ino, S_IFDIR), end);
    }

    // subdir
    size_t subpos = 2;
    proc_foreach(dir->subdir) {
        if (subpos == ctx->pos)
            goto_if(!procfs_dir_emit_ent(ctx, ptr), end);
        subpos++;
    }

    // custom
    if (ctx->pos >= subpos)
        if (dir->op->readdir(dir, ctx, ctx->pos - subpos) < 0)
            ctx->stat = ctx_end;

end:
    return 0;
}

int procfs_seekdir(node_t *this, dirctx_t *ctx, size_t *pos)
{
    if (*pos == 0 || *pos == 1) {
        ctx->stat = ctx_pre;
        ctx->pos = *pos;
        return 0;
    }

    proc_entry_t *dir = this->pdata;
    size_t subsz = proc_ent_count(dir);
    if (subsz + 1 >= *pos) {
        ctx->stat = ctx_pre;
        ctx->pos = *pos;
        return 0;
    }

    size_t rela = *pos;
    if (dir->op->seekdir(dir, ctx, &rela) > 0) {
        *pos = rela;
        return 0;
    }

    ctx->stat = ctx_end;
    ctx->pos = subsz + 1;
    return EOF;
}

void *procfs_mmap(node_t *this, vm_region_t *vm)
{
    proc_entry_t *ent = this->pdata;
    if (ent->op->mmap)
        return ent->op->mmap(ent, vm);
    return (void *)((addr_t)-EINVAL);
}

/*
 * /proc/version
 */

#include <textos/utsname.h>

extern utsname_t __kuname;

static int version_read(struct proc_entry *ent, void *buf, size_t siz, size_t offset)
{
    char name[1 << 9];
    struct utsname *u = &__kuname;
    int n = sprintf(name, "%s %s %s %s %s\n",
            u->sysname,
            u->nodename,
            u->release,
            u->version,
            u->machine);
    if (offset >= n)
        return EOF;
    int rem = n - offset;
    if (rem > siz)
        rem = siz;
    memcpy(buf, name + offset, rem);
    return rem;
}

static proc_opts_t version_op = { .read = version_read };

/*
 * procfs root readdir!
 *  - /proc/.
 *  - /proc/..
 *  - /proc/uptime [fixed]
 *  - /proc/<pid>  [dynamic]
 */

#include <textos/task.h>

extern task_t *table[16];

static int root_open(struct proc_entry *root, char *name, struct proc_entry **res)
{
    int pid = 0;
    for (char *p = name ; *p ; p++) {
        if (*p < '0' || *p > '9')
            return -ENOENT;
        pid = pid * 10 + *p - '0';
    }

    task_t *task = task_get(pid);
    if (!task)
        return -ENOENT;

    proc_entry_t *ent = malloc(sizeof(proc_entry_t));
    ent->name = strdup(name);
    ent->mode = 0555;
    ent->is_dir = true;
    ent->parent = root;
    ent->subdir = NULL;
    ent->next = NULL;
    ent->pdata = task;
    return -ENOENT;
}

static int root_readdir(struct proc_entry *root, dirctx_t *ctx, int relative_pos)
{
    int count = 0;
    for (int i = 0 ; i < 16 ; i++) {
        if (table[i]) {
            if (!relative_pos) {
                char name[16];
                sprintf(name, "%d", table[i]->pid);
                if (!procfs_dir_emit(ctx, name, strlen(name), 0, S_IFDIR))
                    break;
                count++;
            } else relative_pos--;
        }
    }
    return !count ? EOF : 0;
}

static int root_seekdir(struct proc_entry *root, dirctx_t *ctx, size_t *relative_pos)
{
    int count = 0;
    for (int i = 0 ; i < 16 ; i++)
        if (table[i])
            count++;
    if (*relative_pos > count - 1)
        return -ENOENT;
    return 0;
}

static proc_opts_t root_op = {
    .open = root_open,
    .readdir = root_readdir,
    .seekdir = root_seekdir,
};

static proc_opts_t def_op;

/*
 * procfs initialization
 */
node_t *__fs_init_procfs()
{
    dev_t *anony = dev_new();
    dev_register_anony(anony);
    __procfs_super.dev = anony;

    proc_entry_t *root = &__procfs_root;
    root->subdir = NULL;
    root->name = "/";
    root->ino = proc_ino_alloc();
    root->mode = 0555;
    root->is_dir = true;
    root->op = &root_op;
    root->parent = NULL;
    root->subdir = NULL;
    root->next = NULL;
    root->pdata = NULL;

    proc_create("version", 0444, NULL, &version_op);
    proc_create("cpuinfo", 0444, NULL, &def_op);
    proc_create("filesystems", 0444, NULL, &def_op);

    return procfs_nodeget(root);
}

#include <textos/noopt.h>

fs_opts_t __procfs_op = {
    procfs_open,
    procfs_ioctl,
    procfs_close,
    noopt,
    procfs_read,
    procfs_write,
    noopt,
    procfs_readdir,
    procfs_seekdir,
    procfs_mmap,
};