#include <textos/dev.h>
#include <textos/fs.h>
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

    if (dev->minor != 1) return;

    list_t *i;
    LIST_FOREACH(i, &dev->subdev)
    {
        initnod(CR(i, devst_t, subdev));
    }
}

void initproc_nod()
{
    node_t *dir;
    struct fs_openctx ctx = {0};
    vfs_open(NULL, "/dev", O_CREAT | O_DIRECTORY, 0755, &dir,
             &ctx); // rwxr-xr-x
    vfs_open(NULL, "/dev/net", O_CREAT | O_DIRECTORY, 0755, &dir,
             &ctx); // rwxr-xr-x
    vfs_open(NULL, "/dev/event", O_CREAT | O_DIRECTORY, 0755, &dir,
             &ctx); // rwxr-xr-x

    list_t *root = dev_get_root();
    if (root == NULL) PANIC("dev root not initialized\n");

    list_t *i;
    LIST_FOREACH(i, root)
    {
        initnod(CR(i, devstp_t, list)->dev);
    }
}
