#include <textos/boot/hal.h>
#include <textos/boot/mb1.h>

#include "common.h"

#define lbase fq(__lbase)
#define vbase fq(__vbase)
#define lend  fq(__lend)

static __bdata multiboot_memory_map_t mmap[128];
static __bdata size_t mmap_max;
static __bdata multiboot_info_t info_copy;

static __bcode void reskern(addr_t kern_start, addr_t kern_end);
static __bcode void *getpage();

static __bcode multiboot_memory_map_t *getexmap()
{
    for (int i = 0; i < sizeof(mmap) / sizeof(mmap[0]); i++) {
        if (!mmap[i].addr) {
            mmap_max = MAX(mmap_max, i);
            return &mmap[i];
        }
    }
    // logically correct!
    __builtin_unreachable();
}

static __bcode void reskern(addr_t kern_start, addr_t kern_end)
{
    kern_start = align_dn(kern_start, PAGE_SIZE);
    kern_end = align_up(kern_end, PAGE_SIZE);

    multiboot_memory_map_t *map = (void *)mmap;
    for (int i = 0; i < mmap_max; i++, map++)
    {
        addr_t map_start = map->addr;
        addr_t map_end   = map->addr + map->len;
        if (map_end <= kern_start || map_start >= kern_end)
            continue;

        if (map_start < kern_start && map_end > kern_end)
        {
            // map 跨过内核，分成两段
            // 先保留低地址段
            map->len = kern_start - map_start;
            multiboot_memory_map_t *newmap = getexmap();
            newmap->addr = kern_end;
            newmap->len  = map_end - kern_end;
            newmap->type = MULTIBOOT_MEMORY_AVAILABLE;
        }
        else if (map_start >= kern_start && map_end <= kern_end)
        {
            // 整个 map 都被内核占用
            map->type = 0; // 标记为不可用
        }
        else if (map_start < kern_start && map_end > kern_start)
        {
            // map 末尾和内核重叠
            map->len = kern_start - map_start;
        }
        else if (map_start < kern_end && map_end > kern_end)
        {
            // map 起始和内核重叠
            map->addr = kern_end;
            map->len  = map_end - kern_end;
        }
    }
}

static __bcode void *getpage()
{
    multiboot_memory_map_t *map = (void *)mmap;
    for (int i = 0; i < mmap_max; i++, map++)
    {
        if (!map->addr || !map->len)
            continue;
        if (map->type == MULTIBOOT_MEMORY_AVAILABLE)
        {
            addr_t res = map->addr;
            map->addr += PAGE_SIZE;
            map->len -= PAGE_SIZE;
            return (void *)res;
        }
    }
    return NULL;
}

__bcode void __multi64(long magic, long info)
{
    memcpy(&info_copy, (void *)info, sizeof(info_copy));
    info = (long)&info_copy;

    multiboot_info_t *mi = (void *)info;
    memcpy(mmap, (void *)(long)mi->mmap_addr, mi->mmap_length);
    mmap_max = mi->mmap_length / sizeof(multiboot_memory_map_t);

    addr_t kern_start = lbase;
    addr_t kern_end   = align_up(lend, PAGE_SIZE);
    reskern(kern_start, kern_end);
    reskern(mi->cmdline, mi->cmdline + strlen((char *)(long)mi->cmdline));

    mi->mmap_addr = (long)&mmap;
    mi->mmap_length = (long)mmap_max * sizeof(multiboot_memory_map_t);

    __common64(magic, info, getpage, 0);
}
