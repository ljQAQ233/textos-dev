#include <textos/dev.h>
#include <textos/klib/list.h>
#include <textos/klib/string.h>

static void *dev_mmap_stub(devst_t *dev, vm_region_t *vm, ...)
{
    return MAP_FAILED;
}

static void dev_mkname_stub(devst_t *dev, char res[32], int nr)
{}

/*
  The flow to register:
    1. dev_new() to create new buffer
    2. Set it by device initializer itself
    3. dev_register() to insert into the root list
*/

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
        return;
    else if (dev->type == DEV_NET)
        sprintf(path, "/dev/net/%s", dev->name);
    else if (dev->subtype == DEV_EVENT)
        sprintf(path, "/dev/event/%s", dev->name);
    else
        sprintf(path, "/dev/%s", dev->name);

    int mt = 0;
    switch (dev->type) {
    case DEV_CHAR:
        mt = S_IFCHR;
        break;
    case DEV_BLK:
        mt = S_IFBLK;
        break;
    case DEV_NET:
        mt = S_IFSOCK;
        break;
    default:
        break;
    }
    vfs_mknod(path, makedev(dev->major, dev->minor), 0744 | mt);

    DEBUGK(K_INFO, "init dev at %s\n", path);

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
    struct fs_openctx ctx = {0};
    vfs_open(NULL, "/dev", O_CREAT | O_DIRECTORY, 0755, &dir, &ctx);     // rwxr-xr-x
    vfs_open(NULL, "/dev/net", O_CREAT | O_DIRECTORY, 0755, &dir, &ctx); // rwxr-xr-x
    vfs_open(NULL, "/dev/event", O_CREAT | O_DIRECTORY, 0755, &dir, &ctx); // rwxr-xr-x

    list_t *i;
    LIST_FOREACH(i, &root)
    {
        initnod(CR(i, devstp_t, list)->dev);
    }
}

void __dev_register(devstp_t *pri)
{
    if (pri->dev->read == NULL) pri->dev->read = noopt;
    if (pri->dev->write == NULL) pri->dev->write = noopt;
    if (pri->dev->mmap == NULL) pri->dev->mmap = dev_mmap_stub;
    if (pri->dev->ioctl == NULL) pri->dev->ioctl = noopt;
    if (pri->dev->mkname == NULL) pri->dev->mkname = dev_mkname_stub;
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

    d->_init_pctx = NULL;
    d->_fini_pctx = NULL;
    d->read = noopt;  // bread
    d->write = noopt; // bwrite
    d->mmap = dev_mmap_stub;
    d->ioctl = noopt;
    d->mkname = dev_mkname_stub;
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
            if (idx-- == 0) return pri->dev;
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
        if (dm->major != major) continue;
        if (minor == 1) return pm->dev;
        LIST_FOREACH(i, &pm->dev->subdev)
        {
            d = CR(i, devst_t, subdev);
            if (d->minor == minor) return d;
        }
    }

    return NULL;
}

#include <textos/printk.h>

static char *dev_typestr(int type)
{
    if (type == DEV_CHAR) return "character device";
    if (type == DEV_BLK) return "block device";
    return "unknown device";
}

void dev_list()
{
    list_t *i;
    int idx = 0;

    LIST_FOREACH(i, &root)
    {
        devstp_t *pri = CR(i, devstp_t, list);
        printk("dev index - %04d -> %s\n", idx, pri->dev->name);
        printk("            type -> %s\n", dev_typestr(pri->dev->type));
        printk("            opts -> %d%d\n", pri->dev->read == noopt ? 0 : 1,
               pri->dev->write == noopt ? 0 : 1);
        idx++;
    }
}

