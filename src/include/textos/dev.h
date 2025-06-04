#ifndef __DEV_H__
#define __DEV_H__

#include <textos/mm/mman.h>
#include <textos/klib/list.h>

enum dev_type {
    DEV_NONE,
    DEV_CHAR,
    DEV_BLK,
    DEV_NET,
};

enum sub_type {
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

struct dev;
typedef struct dev dev_t;

struct dev {
    char  *name;
    int    type;
    int    subtype;

    uint major;
    uint minor;
    list_t subdev;

    union {
        struct {
            int   (*write)(dev_t *dev, void *buf, size_t cnt);
            int   (*read)(dev_t *dev, void *buf, size_t cnt);
        };
        struct {
            int   (*bwrite)(dev_t *dev, u64 addr, void *buf, size_t cnt);
            int   (*bread)(dev_t *dev, u64 addr, void *buf, size_t cnt);
        };
    };
    void *(*mmap)(dev_t *dev, vm_region_t *vm);
    int (*ioctl)(dev_t *dev, int req, void *argp);
    void (*mkname)(dev_t *dev, char res[32], int nr);

    /* TODO : device isolation 设备隔离 */

    void *pdata;  // to support multiple devs
    addr_t ptoff; // partition r/w start addr
    addr_t ptend;
};

typedef struct {
    dev_t  *dev;
    list_t list;
} dev_pri_t;

enum Lkup {
    LKUP_TYPE,
    LKUP_NAME,
    LKUP_ID,
};

dev_t *dev_new ();

void __dev_register (dev_pri_t *pri);

void dev_register (dev_t *prt, dev_t *dev);

void dev_register_anony(dev_t *dev);

dev_t *dev_lookup_type (int subtype, int idx);

dev_t *dev_lookup_name (const char *name);

dev_t *dev_lookup_nr(uint major, uint minor);

void dev_list ();

#include <textos/fs.h>

dev_t *register_part(
        dev_t *disk, int nr,
        addr_t ptoff, size_t ptsiz,
        node_t *root
        );

node_t *extract_part(dev_t *part);

#endif
