#include <textos/mm.h>
#include <textos/debug.h>
#include <textos/assert.h>

#include <string.h>

static u64 page_total;
static u64 page_free;

/* Mark which zones are free,every node is so small that can be placed in a page easily */
struct free {
    size_t pages;
    struct free *next;
};
typedef struct free free_t;

static free_t _free;

/* 获取 Num 页的内存,没有则返回 NULL */
void *pmm_allocpages (size_t num)
{
    void *page = NULL;

    free_t *n = &_free;
    free_t *prev = &_free;

    do {
        n = n->next;
        if (n->pages >= num)
        {
            n->pages -= num;
            page = (void *)n;

            if (n->pages == 0) {
                prev->next = n->next; // 越过,销毁
            } else {
                free_t *new = OFFSET(n, num * PAGE_SIZ); // 偏移 Node

                new->next = n->next;
                new->pages = n->pages; // 之前已经减过了

                prev->next = new;
            }

            break;
        }
        prev = prev->next;
    } while (n->next);

    if (page) {
        page_free -= num;
        DEBUGK(K_MM, "allocate pages ! - %p,%llu\n", page, num);
    }
    return page;
}

void pmm_allochard(void *page, size_t num)
{
    if (num == 0) return;

    addr_t start = (addr_t)page;
    addr_t end   = start + num * PAGE_SIZ;

    free_t *n = &_free;
    free_t *prev = &_free;

    do {
        n = n->next;

        addr_t ps = (addr_t)n,
               pe = (addr_t)n + n->pages * PAGE_SIZ;
        if (pe < start || end < ps)
            goto next;

        if (start <= ps && pe <= end) {
            prev->next = n->next;
        }

        if (start <= ps && ps < end) {
            free_t *new = (free_t *)end;

            new->pages = (pe - end) / PAGE_SIZ;
            new->next = n->next;
            prev->next = new;
        } else if (start < pe && pe <= end) {
            n->pages -= (pe - start) / PAGE_SIZ;
        } else if (ps < start && end < pe) {
            free_t *free0 = (free_t *)ps;
            free_t *free1 = (free_t *)end;

            size_t pg0 = (start - ps) / PAGE_SIZ;
            size_t pg1 = (pe - end) / PAGE_SIZ;

            free0->pages = pg0;
            free1->pages = pg1;

            free1->next = n->next;
            free0->next = free1;
            prev->next = free0;
        }

next:
        prev = prev->next;
    } while (n->next);
}

static bool _pmm_isfree(void *page, size_t num)
{
    free_t *n = &_free;

    /*
      一句话 : 交集
       ZoneA : ---------xxxxxxxxx
       ZoneB : xxxxxxxxx---------

       non   : ZAStart >= ZBEnd || ZBStart >= ZAEnd
       exist(for `if`) : MAX(ZAStart, ZBStart) < MIN(ZAEnd, ZBEnd)
    */

    addr_t start = (addr_t)page,
           end   = (addr_t)page + num * PAGE_SIZ;

    do {
        n = n->next;

        addr_t ps = (addr_t)n,
               pe = (addr_t)n + n->pages * PAGE_SIZ;
        if (MAX(ps, start) < MIN(pe, end)) {
            return true;
        }
    } while (n->next);

    return false;
}

void pmm_freepages(void *page, size_t num)
{
    page = (void*)((addr_t)page &~ 0x7ff); // 抹掉低位
    if (_pmm_isfree(page, num)) {
        DEBUGK(K_MM, "these pages are free before! - %p,%llu\n", page, num);
        return;
    }

    addr_t start = (addr_t)page,
           end   = (addr_t)page + num * PAGE_SIZ;

    free_t *n = &_free;
    free_t *prev = &_free;
    do {
        n = n->next;

        addr_t ps = (addr_t)n,
               pe = (addr_t)n + n->pages * PAGE_SIZ;

        if (ps == end)        // Head
        {
            free_t *new = page;
            new->pages = n->pages + num;
            new->next = n->next;
            prev->next = new;
            break;
        }
        else if (pe == start) // Tail
        {
            n->pages += num;
            break;
        }
        else if (ps < start && (addr_t)n->next > start) // Another case
        {
            ASSERTK(end < (addr_t)n->next);

            free_t *new = page;
            new->pages = num;
            new->next = n->next;
            n->next = new;
            break;
        }

        prev = prev->next;
    } while (n->next);

    page_free += num;
    DEBUGK(K_MM, "free pages! - %p,%llu\n", page, num);
}

void pmm_init ()
{
    DEBUGK (K_MM, "pages : - total(%llu), free(%llu)\n", page_total, page_free);
}

//

#include <boot.h>
#include <textos/uefi.h>

void __pmm_pre (mconfig_t *m)
{
    DEBUGK(K_INIT, "early-init physical memory!\n");

    mapinfo_t *info = m->map;
    EFI_MEMORY_DESCRIPTOR *desc = info->maps + info->descsiz; // Skip the first one,its ptr points to NULL.

    page_free = 0;

    free_t *n = &_free;
    for (int i = 1 ; i < info->mapcount ; i++, desc = OFFSET(desc, info->descsiz))
    {
        DEBUGK(K_INIT, "[#%02d] 0x%016llx | 0x%016llx | %s\n", i, desc->PhysicalStart, desc->NumberOfPages, get_uefi_mtstr(desc->Type));

        page_total += desc->NumberOfPages;

        if (desc->Type == EfiBootServicesData ||
            desc->Type == EfiBootServicesCode)
            desc->Type  = EfiConventionalMemory;

        if (desc->Type != EfiConventionalMemory)
            continue;
        page_free += desc->NumberOfPages;

        /* 处理与内核占用的物理内存的重叠区域
         *  TODO - replace it!
        */

        n->next = (free_t *)desc->PhysicalStart;
        n = n->next;
        n->pages = desc->NumberOfPages;
    }

    for (kpgs_t *p = m->kpgs ; p->va ; p++)
        pmm_allochard (p->phy, (p->msiz + PAGE_SIZ - 1) / PAGE_SIZ);
}

