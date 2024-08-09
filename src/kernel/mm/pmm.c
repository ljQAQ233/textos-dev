#include <textos/mm.h>
#include <textos/debug.h>
#include <textos/assert.h>

#include <string.h>

static u64 page_total;
static u64 page_free;

/* Mark which zones are free,every node is so small that can be placed in a page easily */
struct free {
    size_t pages;
    addr_t addr;
    struct free *next;
};
typedef struct free free_t;

static free_t _free;

/*
   Ways to allocate memory for nodes:
   1. Put FreeNode in free pages directly
   2. malloc!!!

   alloc  是在原有的基础上进行修改, 如果重映射还没完成, alloc 会将原来在已经分配页上的 节点向后移动一段
   alloc0 在重映射完成后, 是直接 malloc 一个新的节点, 原来的节点应该使用 delete0 来删掉
   二者的行为在重映射完成之前没有任何的差异.
*/

static void *alloc_early (free_t *prev, size_t num) { return OFFSET(prev, num * PAGE_SIZ); }
static void delete_early (free_t *node) { ; }

static void *alloc_post (free_t *prev, size_t num) { return (void *)prev; }
static void delete_post (free_t *node) { ; }

static void *alloc0_post (free_t *prev, size_t num) { return malloc (sizeof(free_t)); }
static void delete0_post (free_t *node) { free (node); }

static void *(*alloc)(free_t *prev, size_t num);
static void *(*alloc0)(free_t *prev, size_t num);
static void *(*delete)(free_t *node);
static void *(*delete0)(free_t *node);

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
            page = (void *)n->addr;

            if (n->pages == 0) {
                prev->next = n->next; // 越过,销毁
                delete0(n);
            } else {
                free_t *new = alloc(n, num);
                new->addr = n->addr + num * PAGE_SIZ;
                new->pages = n->pages; // 之前已经减过了

                new->next = n->next;
                prev->next = new;

                delete(n);
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

        addr_t ps = n->addr,
               pe = n->addr + n->pages * PAGE_SIZ;
        if (pe < start || end < ps)
            goto next;

        if (start <= ps && pe <= end) {
            prev->next = n->next;
            delete0(n);
        }

        if (start <= ps && ps < end) {
            free_t *new = alloc0((free_t *)end, 0);

            new->pages = (pe - end) / PAGE_SIZ;
            new->next = n->next;
            prev->next = new;

            delete0(n);
        } else if (start < pe && pe <= end) {
            n->pages -= (pe - start) / PAGE_SIZ;
        } else if (ps < start && end < pe) {
            free_t *free0 = alloc0((free_t *)ps, 0);
            free_t *free1 = alloc0((free_t *)end, 0);

            size_t pg0 = (start - ps) / PAGE_SIZ;
            size_t pg1 = (pe - end) / PAGE_SIZ;

            free0->pages = pg0;
            free1->pages = pg1;

            free1->next = n->next;
            free0->next = free1;
            prev->next = free0;

            delete0(n);
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
        addr_t ps = (addr_t)n->addr,
               pe = (addr_t)n->addr + n->pages * PAGE_SIZ;
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

        addr_t ps = n->addr,
               pe = n->addr + n->pages * PAGE_SIZ;

        if (ps == end)        // Head
        {
            free_t *new = alloc(page, 0);
            new->addr = start;
            new->pages = n->pages + num;
            new->next = n->next;
            prev->next = new;

            delete(n);
            break;
        }
        else if (pe == start) // Tail
        {
            n->pages += num;
            break;
        }
        else if (ps < start && (!n->next && n->next->addr > start)) // Another case
        {
            ASSERTK(end < n->next->addr);

            free_t *new = alloc0(page, 0);

            new->addr = start;
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

void __pmm_tovmm ()
{
    /* 必须要注意的是, 在完成之前最好避免执行任何的物理内存分配操作 */

    free_t *old = &_free,
           *new = &_free;

    size_t i = 0,
           cnt = 0;
    for (free_t *n = _free.next ; n ; n = n->next) cnt++;

    void *array[cnt];
    for (i = 0 ; i < cnt ; i++)
        array[i] = malloc(sizeof(free_t));

    i = 0;
    while (old->next && i < cnt) {
        old = old->next;

        new->next = array[i++];
        new = new->next;

        new->addr = old->addr;
        new->pages = old->pages;
    }

    while (i < cnt)
        free (array[i++]);

    /* switch mode */
    alloc = (void *)alloc_post;
    alloc0 = (void *)alloc0_post;
    delete = (void *)delete_post;
    delete0 = (void *)delete0_post;
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
        n->addr = (u64)n;
        n->pages = desc->NumberOfPages;
    }
    
    alloc = (void *)alloc_early;
    alloc0 = (void *)alloc_early;
    delete = (void *)delete_early;
    delete0 = (void *)delete_early;
}

