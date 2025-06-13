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
