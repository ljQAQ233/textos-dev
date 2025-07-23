#ifndef __DEV_H__
#define __DEV_H__

#include <textos/mm/mman.h>
#include <textos/klib/list.h>

enum dev_type
{
    DEV_NONE,
    DEV_CHAR,
    DEV_BLK,
    DEV_NET,
};

enum sub_type
{
    DEV_NULL = 1,
    DEV_ZERO,
    DEV_MEMORY,
    DEV_FBDEV,
    DEV_KBD,    /* PS/2 Keyboard  */
    DEV_SERIAL, /* Serial port    */
    DEV_DBGCON, /* QEMU debugcon */
    DEV_KNCON,  /* Kernel console */
    DEV_TTY,
    DEV_IDE,    /* Integrated Drive Electronics */
    DEV_PART,
    DEV_NETIF,
    DEV_ANONY,
};

// dev structure 名字略微生草
struct devst;
typedef struct devst devst_t;

struct devst
{
    char *name;
    int type;
    int subtype;
    uint major;
    uint minor;
    list_t subdev;

    union {
        struct {
            int (*write)(devst_t *dev, void *buf, size_t cnt);
            int (*read)(devst_t *dev, void *buf, size_t cnt);
        };
        struct {
            int (*bwrite)(devst_t *dev, u64 addr, void *buf, size_t cnt);
            int (*bread)(devst_t *dev, u64 addr, void *buf, size_t cnt);
        };
    };
    void *(*mmap)(devst_t *dev, vm_region_t *vm);
    int (*ioctl)(devst_t *dev, int req, void *argp);
    void (*mkname)(devst_t *dev, char res[32], int nr);

    void *pdata;  // to support multiple devs
    addr_t ptoff; // partition r/w start addr
    addr_t ptend;
};

// dev structure private
typedef struct
{
    devst_t  *dev;
    list_t list;
} devstp_t;

/*
 * device number
 * it may be a bit strange. in original unix systems, dev_t is a 16-bit unsigned integer :
 * dev_t [1] = (major << 8) | minor, and later extended to a 32-bit one whose structure
 * is : dev_t [2] = (major << 20) | minor, when musl libc provides the version shown subsquently. [3]
 * [3] is not applied to some filesystems like ext2 whose dev number field is 32-bit. so kernel
 * would use new_decode_dev to convert a uint to quadword dev_t, and use new_encode_dev inversely
 */
static inline unsigned major(dev_t x)
{
    return (unsigned)(((x >> 32) & 0xfffff000) | ((x >> 8) & 0xfff));
}

static inline unsigned minor(dev_t x)
{
    return (unsigned)(((x >> 12) & 0xffffff00) | (x & 0xff));
}

static inline dev_t makedev(unsigned x, unsigned y)
{
    return (dev_t)
       ((x & 0xfffff000ULL) << 32) | ((x & 0xfffULL) << 8) |
       ((y & 0xffffff00ULL) << 12) | ((y & 0xffULL));
}

static inline dev_t makedev_for(devst_t *d)
{
    return makedev(d->major, d->minor);
}

#define NODEV 0

devst_t *dev_new();
void __dev_register(devstp_t *pri);
void dev_register(devst_t *prt, devst_t *dev);
void dev_register_anony(devst_t *dev);

devst_t *dev_lookup_type(int subtype, int idx);
devst_t *dev_lookup_name(const char *name);
devst_t *dev_lookup_nr(uint major, uint minor);

void dev_list();

#include <textos/fs.h>

devst_t *register_part(
        devst_t *disk, int nr,
        addr_t ptoff, size_t ptsiz,
        node_t *root
        );

node_t *extract_part(devst_t *part);

#endif
