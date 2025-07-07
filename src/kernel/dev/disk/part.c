#include <textos/fs.h>
#include <textos/dev.h>
#include <textos/args.h>
#include <textos/errno.h>
#include <textos/klib/vsprintf.h>

#include <string.h>

int part_read(devst_t *dev, u32 addr, void *buf, u8 cnt)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    cnt = MIN(cnt, dev->ptend - addr);
    return prt->bread(prt, addr + dev->ptoff, buf, cnt);
}

int part_write(devst_t *dev, u32 addr, void *buf, u8 cnt)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    cnt = MIN(cnt, dev->ptend - addr);
    return prt->bwrite(prt, addr + dev->ptoff, buf, cnt);
}

int part_ioctl(devst_t *dev, int req, void *argp)
{
    devst_t *prt = dev_lookup_nr(dev->major, 1);
    switch (req)
    {
    case BLKSSZGET:
        return prt->ioctl(dev, req, argp);
    default:
        break;
    }
    return -EINVAL;
}

devst_t *register_part(
        devst_t *disk, int nr,
        addr_t ptoff, size_t ptsiz,
        node_t *root
        )
{
    char name[32];
    disk->mkname(disk, name, nr);
    
    devst_t *part = dev_new();
    part->name = strdup(name);
    part->type = DEV_BLK;
    part->subtype = DEV_PART;
    part->bread = (void *)part_read;
    part->bwrite = (void *)part_write;
    part->ioctl = (void *)part_ioctl;
    part->ptoff = ptoff;
    part->ptend = ptoff + ptsiz;
    part->pdata = root;
    dev_register(disk, part);

    return part;
}

node_t *extract_part(devst_t *part)
{
    return part->pdata;
}
