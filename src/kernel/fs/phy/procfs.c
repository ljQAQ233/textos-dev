/*
 * procfs - an implementation by textos
 *
 * the procfs provides interfaces to create directory or files in this filesystem for the kernel or
 * device drivers or kernel modules which will be implemented here. the procfs exposes info
 * to the user applications through files which are mostly read-only and others can be written to configure
 * a feature or enable a function.
 *
 * `proc_create` - create a file entry
 * `proc_mkdir` - create a dir entry
 * `proc_remove` - remove a entry and its sub-entries
 * all of them are used by kernel layer.
 *
 * procfs allows every entry to use its own operations, including open / read / readdir (dir)... etc.
 * but there's only one condition allowed to do that: initialized by the kernel
 *
 * the code to achieve that may be simple, but let's  think of an inode number...
 * I have a hard time choosing... I'm so indecisive! (´･ω･`) -> refer to `pid entry 设计`
 *
 * every entry in /proc has its unique inode number. the num of entries created by `proc_create` is allocated
 * automatically. `<pid>` entries have the number format of (pid << 32) | (0).
 *
 * kernel makes sure that the entries created by itself to have a fixed inode number util the entry is destroyed.
 * as regards entries created by others, excluding using `proc_create`, the number may be unspecified.
 */

/*
 * pid entry 设计:
 *  - 1. pid dir 需要注册到 /proc 即直接挂到 __procfs_root 下面吗
 *      - 不要了吧! 用的时候直接在 procfs_open 里面动手脚就可以了
 *  - 2. 如果 readdir /proc/<pid>, 那么就必须需要一个 inode number,
 *       inode number 该保存到什么地方, 还是直接将 这个 entry 实例化
 *      - 实例化 - 多出了一个 proc_entry (N个)
 *      - hashmap 保存 ino - 好像有点麻烦
 *      - 偷鸡摸狗 : 反正 readdir 是我负责了! 就没有 root 的事了吧? (假的)
 *          反正这里是在内核, 那我直接规定就好了.
 *          ino = (pid << 32) | (pid entry index)
 *          - 考虑 /proc/<pid>/fd/ 
 *            一层不行再来一层!
 *            ino1 = ino | (1 << 31) | fd
 *          - 如果又出现一个会变的目录?
 *              hash 总治得了你了吧! 冲突了怎么办? 概率那么小!
 *      * 好吧, 显然最后一种适合我 :)
 *      * 那么, proc_create 创建的文件, 就好好地用 自动分配的 ino 吧!
 *  - 3. proc_opts 需要吗? 直接用 fs_opts 有什么好处?
 */

#include <textos/task.h>
#include <textos/file.h>
#include <textos/errno.h>
#include <textos/noopt.h>
#include <textos/fs.h>
#include <textos/fs/inter.h>
#include <textos/fs/procfs.h>
#include <textos/panic.h>
#include <textos/mm/heap.h>
#include <textos/klib/string.h>
#include <textos/args.h>
#include <textos/klib/vsprintf.h>

static proc_opts_t def_op;

#define goto_if(cond, label) do { if (cond) goto label; } while (0)

bool procfs_dir_emit(dirctx_t *ctx, const char *name, size_t len, u64 ino, unsigned type)
{
    if (dir_emit(ctx, name, len, ino, type))
    {
        ctx->pos++;
        return true;
    }
    return false;
}

static bool procfs_dir_emit_ent(dirctx_t *ctx, proc_entry_t *ent)
{
    if (dir_emit(ctx, ent->name, strlen(ent->name), 0, dir_get_type(ent->mode)))
    {
        ctx->pos++;
        return true;
    }
    return false;
}

static u64 __procfs_ino = 1;
static superblk_t __procfs_super;
static proc_entry_t __procfs_root;

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

static bool proc_is_root(proc_entry_t *ent)
{
    return ent == &__procfs_root;
}

/*
 * ([pid] << 32) | [IFLAG_FD] | [fd number]
 */
#define ISHIFT_PID 32
#define IMASK_LOW  ((1ull << ISHIFT_PID) - 1)
#define IFLAG_FD   (1 << 31)
#define IMASK_FD   (IFLAG_FD - 1)

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

    if (op)
    {
        op->open = noopt;
        op->ioctl = noopt;
        op->close = noopt;
        op->read = noopt;
        op->write = noopt;
        op->truncate = noopt;
        // func readdir here;
        // func seekdir here;
        op->mmap = noopt;
    }
    else
        op = &def_op;

    dir->name = name;
    dir->ino = proc_ino_alloc();
    dir->mode = S_IFDIR | 0555;
    dir->op = op;
    proc_ent_reg(prt, dir);
    return dir;
}

void proc_remove(proc_entry_t *ent)
{
    if (!proc_is_root(ent))
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
    unsigned ma = __procfs_super.dev->major;
    unsigned mi = __procfs_super.dev->minor;

    node_t *node = malloc(sizeof(node_t));
    node->name = strdup(ent->name);
    node->mode = ent->mode;
    node->siz = 0;
    node->ino = ent->ino;
    node->atime = arch_time_now();
    node->mtime = arch_time_now();
    node->ctime = arch_time_now();
    node->parent = NULL;
    node->child = NULL;
    node->next = NULL;
    node->dev = makedev(ma, mi);
    node->rdev = NODEV;
    node->pdata = ent;
    node->sb = &__procfs_super;
    node->mount = NULL;
    node->opts = &__procfs_op;
    return node;
}

static int procfs_open(node_t *parent, char *name, u64 args, int mode, node_t **result)
{
    proc_entry_t *dir = parent->pdata;
    proc_entry_t *chd = proc_ent_find(dir, name);

    if (!chd) {
        int ret = dir->op->open(dir, name, &chd);
        if (ret < 0)
            return ret;
    }

    node_t *node = procfs_nodeget(chd);
    vfs_regst(node, parent);
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

static inline void init_ctx(dirctx_t *ctx, node_t *dir)
{
    ctx->sb = dir->sb;
    ctx->node = dir;
    ctx->pos = 0;
    ctx->bidx = 0;
    ctx->eidx = 0;
    ctx->stat = ctx_pre;
}

static int procfs_readdir(node_t *node, dirctx_t *ctx)
{
    if (ctx->stat == ctx_inv)
        init_ctx(ctx, node);

    proc_entry_t *dir = node->pdata;
    // .
    if (ctx->pos == 0)
    {
        goto_if(!dir_emit_dot(ctx), end);
        ctx->pos++;
    }
    // ..
    if (ctx->pos == 1)
    {
        goto_if(!dir_emit_dotdot(ctx), end);
        ctx->pos++;
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

#define ignore(x) ((void)x)

static size_t proc_submit_buf(void *output, size_t osz, void *buf, size_t bsz, size_t off)
{
    if (off >= bsz)
        return EOF;
    size_t rem = bsz - off;
    if (rem > osz)
        rem = osz;
    memcpy(output, buf + off, rem);
    return rem;
}

static int pid_get(struct proc_entry *pe)
{
    return pe->ino >> ISHIFT_PID;
}

static int proc_printf(void *buf, size_t siz, size_t off, const char *format, ...)
{
    char _[128];
    int n;
    va_list args;
    va_start(args, format);
    n = vsprintf(_, format, args);
    va_end(args);
    return proc_submit_buf(buf, siz, _, n, off);
}

static int pid_pid_read(struct proc_entry *this, void *buf, size_t siz, size_t offset)
{
    int pid = pid_get(this);
    return proc_printf(buf, siz, offset, "%d\n", pid);
}

static int pid_ppid_read(struct proc_entry *this, void *buf, size_t siz, size_t offset)
{
    int pid = pid_get(this);
    int ppid = task_get(pid)->ppid;
    return proc_printf(buf, siz, offset, "%d\n", ppid);
}

static int pid_fpust_read(struct proc_entry *this, void *buf, size_t siz, size_t offset)
{
    int pid = pid_get(this);
    int fpust = task_get(pid)->fpu != NULL;
    return proc_printf(buf, siz, offset, "%d\n", fpust);
}

/*
 * /proc/<pid>
 */
typedef struct pid_entry
{
    const char *name;
    int mode;
    proc_opts_t op;
} pid_entry_t;

#define REG(NAME, MODE, OP) { (NAME), S_IFREG | (MODE), OP }
#define DIR(NAME, MODE, OP) { (NAME), S_IFDIR | (MODE), OP }

static pid_entry_t pid_entry[] = {
    REG("pid",    S_IRUSR | S_IRGRP | S_IROTH, { .read = pid_pid_read   }),
    REG("ppid",   S_IRUSR | S_IRGRP | S_IROTH, { .read = pid_ppid_read  }),
    REG("fpust",  S_IRUSR | S_IRGRP | S_IROTH, { .read = pid_fpust_read }),
};

#define pid_entry_max (sizeof(pid_entry) / sizeof(pid_entry_t))

static int pid_open(struct proc_entry *dir, char *name, struct proc_entry **res)
{
    for (int idx = 0 ; idx < pid_entry_max ; idx++)
    {
        if (strcmp(pid_entry[idx].name, name) == 0)
        {
            pid_entry_t *pe = &pid_entry[idx];
            proc_entry_t *ent = malloc(sizeof(proc_entry_t));
            ent->name = strdup(pe->name);
            ent->ino = (dir->ino &~ IMASK_LOW) | (idx + 1);
            ent->parent = dir;
            ent->subdir = NULL;
            ent->next = NULL;
            ent->op = &pe->op;
            *res = ent;
            return 0;
        }
    }
    return -ENOENT;
}

static int pid_readdir(struct proc_entry *dir, dirctx_t *ctx, int relative_pos)
{
    int count = 0;
    if (relative_pos >= pid_entry_max)
    {
        ctx->stat = ctx_end;
        return EOF;
    }

    for (int idx = relative_pos ; idx < pid_entry_max ; idx++)
    {
        u64 ino = (dir->ino & IMASK_LOW) | (idx + 1);
        const char *name = pid_entry[idx].name;
        unsigned mode = pid_entry[idx].mode;
        if (!procfs_dir_emit(ctx, name, strlen(name), ino, mode))
            break;
        count++;
    }
end:
    return !count ? EOF : count;
}

static int pid_seekdir(struct proc_entry *dir, dirctx_t *ctx, size_t *relative_pos)
{
    if (*relative_pos >= pid_entry_max)
        return -ENOENT;
    return 0;
}

static proc_opts_t pid_op = {
    .open = pid_open,
    .readdir = pid_readdir,
    .seekdir = pid_seekdir,
};

/*
 * /proc/version
 */

#include <textos/utsname.h>

extern utsname_t __kuname;

static int version_read(struct proc_entry *ent, void *buf, size_t siz, size_t offset)
{
    struct utsname *u = &__kuname;
    return proc_printf(buf, siz, offset,
            "%s %s %s %s %s\n",
            u->sysname,
            u->nodename,
            u->release,
            u->version,
            u->machine);
}

static int meow_read(struct proc_entry *this, void *buf, size_t siz, size_t offset)
{
    return proc_printf(buf, siz, offset, "meow~\n");
}

static proc_opts_t version_op = { .read = version_read };
static proc_opts_t catmeow_op = { .read = meow_read    };

/*
 * procfs root readdir!
 *  - /proc/.
 *  - /proc/..
 *  - /proc/uptime [fixed]
 *  - /proc/<pid>  [dynamic]
 */

extern task_t *table[16];

static int root_open(struct proc_entry *root, char *name, struct proc_entry **res)
{
    int pid = 0;
    for (char *p = name ; *p && *p != '/' ; p++) {
        if (*p < '0' || *p > '9')
            return -ENOENT;
        pid = pid * 10 + *p - '0';
    }

    task_t *task = task_get(pid);
    if (!task)
        return -ENOENT;

    proc_entry_t *ent = malloc(sizeof(proc_entry_t));
    ent->name = strdup(name);
    ent->ino = ((u64)pid << ISHIFT_PID) | 0;
    ent->mode = S_IFDIR | 0555;
    ent->parent = root;
    ent->subdir = NULL;
    ent->next = NULL;
    ent->op = &pid_op;
    *res = ent;
    return 0;
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

/*
 * procfs initialization
 */
node_t *__fs_init_procfs()
{
    devst_t *anony = dev_new();
    dev_register_anony(anony);
    superblk_t *sb = &__procfs_super;
    sb->blksz = PAGE_SIZ;
    sb->dev = anony;
    sb->root = NULL;
    sb->op = &__procfs_op;
    sb->sbi = NULL;

    proc_entry_t *root = &__procfs_root;
    root->name = "/proc";
    root->ino = proc_ino_alloc();
    root->mode = S_IFDIR | 0555;
    root->op = &root_op;
    root->parent = NULL;
    root->subdir = NULL;
    root->next = NULL;
    root->pdata = NULL;

    proc_create("meow",        S_IRUSR | S_IRGRP | S_IROTH, NULL, &catmeow_op);
    proc_create("version",     S_IRUSR | S_IRGRP | S_IROTH, NULL, &version_op);
    proc_create("cpuinfo",     S_IRUSR | S_IRGRP | S_IROTH, NULL, &def_op);
    proc_create("filesystems", S_IRUSR | S_IRGRP | S_IROTH, NULL, &def_op);

    return sb->root = procfs_nodeget(root);
}

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
