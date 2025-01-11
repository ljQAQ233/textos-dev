#ifndef __DEV_H__
#define __DEV_H__

#include <textos/klib/list.h>

enum dev_type {
    DEV_CHAR,
    DEV_BLK,
};

enum sub_type {
    DEV_NULL = 1,
    DEV_ZERO,
    DEV_KBD,    /* PS/2 Keyboard  */
    DEV_SERIAL, /* Serial port    */
    DEV_KNCON,  /* Kernel console */
    DEV_IDE,    /* Integrated Drive Electronics */
};

struct dev;
typedef struct dev dev_t;

struct dev {
    char  *name;
    int    type;
    int    subtype;

    int major;
    int minor;
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

    /* TODO : device isolation 设备隔离 */

    void *pdata; /* 在调用 操作函数 时传入, 以支持同种类型的多种设备 */
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

dev_t *dev_lookup_type (int subtype, int idx);

dev_t *dev_lookup_name (const char *name);

dev_t *dev_lookup_nr (int major, int minor);

void dev_list ();

#endif
