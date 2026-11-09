#include <textos/dev.h>
#include <textos/klib/list.h>
#include <textos/klib/string.h>
#include <textos/mm/mman.h>
#include <textos/printk.h>

static void *dev_mmap_stub(devst_t *dev, vm_region_t *vm, ...)
{
    return MAP_FAILED;
}

static void dev_mkname_stub(devst_t *dev, char res[32], int nr)
{}

static int dev_poll_setup_stub(devst_t *dev, struct fs_poller *poller)
{
    return 1;
}

static int dev_poll_teardown_stub(devst_t *dev, struct fs_poller *poller)
{
    return 1;
}

static dev_opts_t stub_op = {
    ._init_pctx = NULL,
    ._fini_pctx = NULL,
    .read = noopt,  // bread
    .write = noopt, // bwrite
    .mmap = dev_mmap_stub,
    .ioctl = noopt,
    .mkname = dev_mkname_stub,
    .poll_setup = dev_poll_setup_stub,
    .poll_teardown = dev_poll_teardown_stub,
    .driver_name = "stub",
};

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

    dev_debug_list();
}

void dev_stub_padops(dev_opts_t *op)
{
    // _init_pctx keeps NULL
    // _fini_pctx keeps NULL
    if (op->read == NULL) op->read = noopt;
    if (op->write == NULL) op->write = noopt;
    if (op->mmap == NULL) op->mmap = dev_mmap_stub;
    if (op->ioctl == NULL) op->ioctl = noopt;
    if (op->mkname == NULL) op->mkname = dev_mkname_stub;
    if (op->driver_name == NULL) op->driver_name = "stub";
}

void dev_stub_getops(dev_opts_t **op)
{
    *op = &stub_op;
}

void __dev_register(devstp_t *pri)
{
    if (pri->dev->op)
        dev_stub_padops(pri->dev->op);
    else
        pri->dev->op = &stub_op;
    list_init(&pri->dev->subdev);
    list_insert_after(&root, &pri->list);
}

static uint applyid(devst_t *prt)
{
    static uint total = 1;
    if (!prt) return total++;
    return CR(prt->subdev.next, devst_t, subdev)->minor + 1;
}

void dev_register(devst_t *prt, devst_t *dev)
{
    if (prt != NULL) {
        dev->major = prt->major;
        if (!dev->minor) dev->minor = applyid(prt);
        list_insert(&prt->subdev, &dev->subdev);
        return;
    }

    if (!dev->major) dev->major = applyid(NULL);
    dev->minor = 1;
    list_init(&dev->subdev);

    devstp_t *pri = malloc(sizeof(devstp_t));
    pri->dev = dev;

    __dev_register(pri);
}

devst_t *dev_new()
{
    devst_t *d = malloc(sizeof(devst_t));
    d->op = &stub_op;
    d->major = 0;
    d->minor = 0;
    return d;
}

list_t *dev_get_root()
{
    return &root;
}

devst_t *dev_lookup_type(int subtype, int idx)
{
    list_t *i, *j;
    LIST_FOREACH(i, &root)
    {
        devstp_t *pri = CR(i, devstp_t, list);
        if (pri->dev->subtype == subtype)
            if (idx-- == 0) return pri->dev;
        LIST_FOREACH(j, &pri->dev->subdev)
        {
            devst_t *dj = CR(j, devst_t, subdev);
            if (dj->subtype == subtype)
                if (idx-- == 0) return dj;
        }
    }
    return NULL;
}

devst_t *dev_lookup_name(const char *name)
{
    list_t *i, *j;
    LIST_FOREACH(i, &root)
    {
        devstp_t *pri = CR(i, devstp_t, list);
        if (strcmp(pri->dev->name, name) == 0) return pri->dev;
        LIST_FOREACH(j, &pri->dev->subdev)
        {
            devst_t *dj = CR(j, devst_t, subdev);
            if (strcmp(dj->name, name) == 0) return dj;
        }
    }

    return NULL;
}

devst_t *dev_lookup_nr(uint major, uint minor)
{
    list_t *im, *i;  // iterator
    devstp_t *pm;    // private
    devst_t *dm, *d; // device

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

static char *dev_typestr(int type)
{
    if (type == DEV_CHAR) return "c";
    if (type == DEV_BLK) return "b";
    if (type == DEV_NET) return "n";
    return "u";
}

#define statof(x, c) (((x) == noopt || !(x)) ? '-' : (c))

static void dev_info_dump(devst_t *dev)
{
    DEBUGK(K_DEBUG | K_CONT, "%- 4s type=%s opts=%c%c%c%c%c\n", dev->name,
           dev_typestr(dev->type), statof(dev->op->read, 'r'),
           statof(dev->op->write, 'w'), statof(dev->op->ioctl, 'i'),
           statof(dev->op->mmap, 'm'), statof(dev->op->poll_setup, 'p'));
}

void dev_debug_list()
{
    list_t *i, *j;

    LIST_FOREACH(i, &root)
    {
        devstp_t *pri = CR(i, devstp_t, list);
        dev_info_dump(pri->dev);
        LIST_FOREACH(j, &pri->dev->subdev)
        {
            devst_t *dj = CR(j, devst_t, subdev);
            dev_info_dump(dj);
        }
    }
}
