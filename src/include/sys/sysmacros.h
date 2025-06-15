#ifndef _SYS_SYSMACROS_H
#define _SYS_SYSMACROS_H

#include <sys/cdefs.h>

__BEGIN_DECLS

/* os-specified */

#define __NEED_dev_t
#include <bits/alltypes.h>

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

__END_DECLS

#endif