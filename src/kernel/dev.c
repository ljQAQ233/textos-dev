#include <textos/dev.h>
#include <textos/mm.h>
#include <textos/panic.h>
#include <textos/klib/list.h>

#include <string.h>

/*
  The flow to register:
    1. dev_new() to create new buffer
    2. Set it by device initializer itself
    3. dev_register() to insert into the root list
*/

static void inv_handle()
{
    PANIC ("this opts is not supported!");
}

static list_t root = LIST_INIT(root);

extern void __dev_initmem();
extern void __dev_initanony();
extern void __dev_initdbgcon();

void dev_init()
{
    __dev_initmem();
    __dev_initanony();
    __dev_initdbgcon();

    dev_list();
}

#include <textos/fs.h>
#include <textos/args.h>
#include <textos/klib/vsprintf.h>

static void initnod(devst_t *dev)
{
    char path[64];
    if (dev->subtype == DEV_ANONY)
        return ;
    if (dev->type == DEV_NET)
        sprintf(path, "/dev/net/%s", dev->name);
    else
        sprintf(path, "/dev/%s", dev->name);
    vfs_mknod(path, dev);

    DEBUGK(K_SYNC, "init dev at %s\n", path);

    if (dev->minor != 1)
        return;

    list_t *i;
    LIST_FOREACH(i, &dev->subdev)
    {
        initnod(CR(i, devst_t, subdev));
    }
}

void dev_initnod()
{
    node_t *dir;
    vfs_open(NULL, &dir, "/dev", VFS_CREATE | VFS_DIR);
    vfs_open(NULL, &dir, "/dev/net", VFS_CREATE | VFS_DIR);

    list_t *i;
    LIST_FOREACH(i, &root)
    {
        initnod(CR(i, devstp_t, list)->dev);
    }
}

void __dev_register(devstp_t *pri)
{
    if (pri->dev->read == NULL)
        pri->dev->read = (void *)inv_handle;
    if (pri->dev->write == NULL)
        pri->dev->write = (void *)inv_handle;
    list_init(&pri->dev->subdev);
    
    list_insert_after(&root, &pri->list);
}

static uint applyid(devst_t *prt)
{
    static uint total = 1;
    if (!prt)
        return total++;
    return CR(prt->subdev.next, devst_t, subdev)->minor + 1;
}

void dev_register (devst_t *prt, devst_t *dev)
{
    if (prt != NULL)
    {
        dev->major = prt->major;
        if (!dev->minor)
            dev->minor = applyid(prt);
        list_insert(&prt->subdev, &dev->subdev);
        return;
    }

    if (!dev->major)
        dev->major = applyid(NULL);
    dev->minor = 1;
    list_init(&dev->subdev);

    devstp_t *pri = malloc(sizeof(devstp_t));
    pri->dev = dev;

    __dev_register(pri);
}

devst_t *dev_new()
{
    devst_t *d = malloc(sizeof(devst_t));
    
    d->read = (void *)inv_handle;
    d->write = (void *)inv_handle;
    d->bread = (void *)inv_handle;
    d->bwrite = (void *)inv_handle;
    d->major = 0;
    d->minor = 0;
    
    return d;
}

devst_t *dev_lookup_type(int subtype, int idx)
{
    list_t *i;
    LIST_FOREACH(i, &root)
    {
        devstp_t *pri = CR(i, devstp_t, list);
        if (pri->dev->subtype == subtype)
            if (idx-- == 0)
                return pri->dev;
    }

    return NULL;
}

devst_t *dev_lookup_name(const char *name)
{
    list_t *i;
    LIST_FOREACH(i, &root)
    {
        devstp_t *pri = CR(i, devstp_t, list);
        if (strcmp(pri->dev->name, name) == 0)
            return pri->dev;
    }

    return NULL;
}

devst_t *dev_lookup_nr(uint major, uint minor)
{
    list_t *im, *i; // iterator
    devstp_t *pm;  // private
    devst_t *dm, *d;  // device

    LIST_FOREACH(im, &root)
    {
        pm = CR(im, devstp_t, list);
        dm = pm->dev;
        if (dm->major != major)
            continue;
        if (minor == 1)
            return pm->dev;
        LIST_FOREACH(i, &pm->dev->subdev)
        {
            d = CR(i, devst_t, subdev);
            if (d->minor == minor)
                return d;
        }
    }

    return NULL;
}

#include <textos/printk.h>

static char *dev_typestr(int type)
{
    if (type == DEV_CHAR)
        return "character device";
    if (type == DEV_BLK)
        return "block device";

    return "unknown device";
}

void dev_list()
{
    list_t *i;
    int idx = 0;

    LIST_FOREACH(i, &root)
    {
        devstp_t *pri = CR (i, devstp_t, list);
        printk ("dev index - %04d -> %s\n"  , idx, pri->dev->name);
        printk ("            type -> %s\n"  , dev_typestr(pri->dev->type));
        printk ("            opts -> %d%d\n",
                pri->dev->read == (void *)inv_handle ? 0 : 1,
                pri->dev->write == (void *)inv_handle ? 0 : 1);

        idx++;
    }
}

