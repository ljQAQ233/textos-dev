#pragma once

struct proc_opts;
struct proc_entry;

/*
 * for a regular file, any of op may be nullptr, but for a dir registered by
 * `proc_mkdir`, op would be either a op function or `noopt`. btw, `proc_mkdir`
 * would set all nullptr to `noopt` in passing.
 */
typedef struct proc_opts
{
    int  (*open)(struct proc_entry *parent, char *name, struct proc_entry **res);
    int  (*ioctl)(struct proc_entry *this, int req, void *argp);
    int  (*close)(struct proc_entry *this);
    int  (*destroy)(struct proc_entry *this); // TODO
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
    const proc_opts_t *op;
    struct proc_entry *parent;
    struct proc_entry *subdir;
    struct proc_entry *next;
    void *pdata;
} proc_entry_t;

proc_entry_t *proc_create(char *name, int mode, proc_entry_t *prt, proc_opts_t *op);

proc_entry_t *proc_mkdir(char *name, proc_entry_t *prt, proc_opts_t *op);

void proc_remove(proc_entry_t *ent);
