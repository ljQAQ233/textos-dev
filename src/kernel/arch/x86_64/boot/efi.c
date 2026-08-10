#include <textos/boot.h>
#include <textos/user/elf.h>
#include <textos/boot/efi.h>
#include <textos/uefi.h>

#include "common.h"
#include "textos/uefi/mm.h"

static __bdata void *descs;
static __bdata size_t desccnt;
static __bdata size_t descsiz;

#define to_len(pgs) (EFI_PAGE_SIZE * (pgs))
#define to_pgs(len) ((len) / EFI_PAGE_SIZE)

static __bcode void reskern(addr_t kern_start, addr_t kern_end)
{
    for (int i = 0; i < desccnt; i++)
    {
        EFI_MEMORY_DESCRIPTOR *desc = (void *)descs + descsiz * i;
        addr_t start = desc->PhysicalStart;
        addr_t end   = desc->PhysicalStart + to_len(desc->NumberOfPages);
        if (end <= kern_start || start >= kern_end)
            continue;

        if (start < kern_start && end > kern_end)
        {
            // map 跨过内核，分成两段
            // 先保留低地址段
            desc->NumberOfPages = kern_start - start;
            EFI_MEMORY_DESCRIPTOR *newmap = (EFI_MEMORY_DESCRIPTOR *)descs;
            newmap->PhysicalStart = kern_end;
            newmap->NumberOfPages = to_pgs(end - kern_end);
            newmap->Type = desc->Type;
            newmap->Attribute = desc->Attribute;
        }
        else if (start >= kern_start && end <= kern_end)
        {
            // 整个 map 都被内核占用
            desc->PhysicalStart = 0;
            desc->NumberOfPages = 0;
        }
        else if (start < kern_start && end > kern_start)
        {
            // map 末尾和内核重叠
            desc->NumberOfPages = to_pgs(kern_start - start);
        }
        else if (start < kern_end && end > kern_end)
        {
            // map 起始和内核重叠
            desc->PhysicalStart = kern_end;
            desc->NumberOfPages  = to_pgs(end - kern_end);
        }
    }
}

static __bcode void *getpage()
{
    EFI_MEMORY_DESCRIPTOR *map = (void *)descs;
    for (int i = 0; i < desccnt; i++, map++)
    {
        if (!map->PhysicalStart || !map->NumberOfPages)
            continue;
        if (map->Type == EfiConventionalMemory)
        {
            addr_t res = map->PhysicalStart;
            map->PhysicalStart += to_len(1);
            map->NumberOfPages -= 1;
            return (void *)res;
        }
    }
    return NULL;
}

__bcode void __efi64(long magic, long info)
{
    bconfig_t *b = (bconfig_t *)info;
    mapinfo_t *i = b->mapinfo;
    descs = i->map;
    desccnt = i->desccnt;
    descsiz = i->descsiz;
    reskern(b->load_base, b->load_base + b->load_size);

    // expect + offset = real
    addr_t expect_entry = fq(_start);
    addr_t real_entry = b->phy_entry;
    addr_t offset = real_entry - expect_entry;
    __common64(magic, info, getpage, offset);
}
