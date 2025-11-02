#include <textos/mm.h>
#include <textos/mm/vmm.h>
#include <textos/assert.h>
#include <textos/klib/string.h>

static u64 page_total;
static u64 page_free;

/* Mark which zones are free,every node is so small that can be placed in a page easily */
typedef struct free
{
    size_t pages;
    addr_t addr;
    struct free *next;
} free_t;

static free_t _free;

/*
   Ways to allocate memory for nodes:
   1. Put FreeNode in free pages directly
   2. malloc!!!

   alloc  是在原有的基础上进行修改, 如果重映射还没完成, alloc 会将原来在已经分配页上的 节点向后移动一段
   alloc0 在重映射完成后, 是直接 malloc 一个新的节点, 原来的节点应该使用 delete0 来删掉
   二者的行为在重映射完成之前没有任何的差异.
*/

static void *alloc_early(free_t *prev, size_t num) { return OFFSET(prev, num * PAGE_SIZ); }
static void delete_early(free_t *node) { }
static void *alloc_post(free_t *prev, size_t num) { return (void *)prev; }
static void delete_post(free_t *node) { }
static void *alloc0_post(free_t *prev, size_t num) { return malloc (sizeof(free_t)); }
static void delete0_post(free_t *node) { free (node); }

static void *(*alloc)(free_t *prev, size_t num);
static void *(*alloc0)(free_t *prev, size_t num);
static void (*delete)(free_t *node);
static void (*delete0)(free_t *node);

/* 获取 Num 页的内存,没有则返回 NULL */
addr_t pmm_allocpages(size_t num)
{
    addr_t page = 0;
    free_t *n = &_free;
    free_t *prev = &_free;
    do
    {
        n = n->next;
        if (n->pages >= num)
        {
            n->pages -= num;
            page = n->addr;
            if (n->pages == 0)
            {
                prev->next = n->next; // 越过,销毁
                delete0(n);
            }
            else
            {
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

    if (page)
    {
        page_free -= num;
        DEBUGK(K_MM, "allocate pages ! - %p,%llu\n", page, num);
    }
    return page;
}

void pmm_allochard(addr_t page, size_t num)
{
    if (num == 0)
        return;

    addr_t start = (addr_t)page;
    addr_t end = start + num * PAGE_SIZ;
    free_t *n = &_free;
    free_t *prev = &_free;
    do
    {
        n = n->next;

        addr_t ps = n->addr, pe = n->addr + n->pages * PAGE_SIZ;
        if (pe < start || end < ps)
            goto next;

        if (start <= ps && pe <= end)
        {
            prev->next = n->next;
            delete0(n);
        }

        if (start <= ps && ps < end)
        {
            free_t *new = alloc0((free_t *)end, 0);

            new->pages = (pe - end) / PAGE_SIZ;
            new->next = n->next;
            prev->next = new;

            delete0(n);
        }
        else if (start < pe && pe <= end)
        {
            n->pages -= (pe - start) / PAGE_SIZ;
        }
        else if (ps < start && end < pe)
        {
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

static bool _pmm_isfree(addr_t page, size_t num)
{
    free_t *n = &_free;
    /*
     *  ZoneA : ---------xxxxxxxxx
     *  ZoneB : xxxxxxxxx---------
     */
    addr_t start = (addr_t)page;
    addr_t end   = (addr_t)page + num * PAGE_SIZ;
    do {
        n = n->next;
        addr_t ps = (addr_t)n->addr,
               pe = (addr_t)n->addr + n->pages * PAGE_SIZ;
        if (MAX(ps, start) < MIN(pe, end))
            return true;
    } while (n->next);
    return false;
}

void pmm_freepages(addr_t page, size_t num)
{
    page = (addr_t)page &~ PAGE_MASK;
    ASSERTK(!_pmm_isfree(page, num));
    addr_t start = page;
    addr_t end = page + num * PAGE_SIZ;
    free_t *n = &_free;
    free_t *prev = &_free;
    do
    {
        n = n->next;
        addr_t ps = n->addr;
        addr_t pe = n->addr + n->pages * PAGE_SIZ;
        if (ps == end)
        {
            /*
             * use `alloc` to adjust `n` to the start of the page
             * +------+---+ if it is in the physical address mode
             * | page | n | otherwise `n` is not related to the according
             * +------+---+ physical address which is store in heap
             */
            free_t *new = alloc(n, -n->pages);
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
        else if (pe < start && (!n->next || n->next->addr > end))
        {
            /*
             * +-+-+----+-+-----------+
             * |n|1|page|1|next / none|
             * +-+-+----+-+-----------+
             */
            ASSERTK(!n->next || end < n->next->addr);
            free_t *new = alloc0(n, 0);
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

void pmm_init()
{
    DEBUGK(K_MM, "pages : - total(%llu), free(%llu)\n", page_total, page_free);
}

void __pmm_tovmm()
{
    /* 必须要注意的是, 在完成之前最好避免执行任何的物理内存分配操作 */

    free_t *old = &_free, *new = &_free;

    size_t i = 0, cnt = 0;
    for (free_t *n = _free.next; n; n = n->next)
        cnt++;

    void *array[cnt];
    for (i = 0; i < cnt; i++)
        array[i] = malloc(sizeof(free_t));

    i = 0;
    while (old->next && i < cnt)
    {
        old = old->next;

        new->next = array[i++];
        new = new->next;

        new->addr = old->addr;
        new->pages = old->pages;
    }

    while (i < cnt)
        free(array[i++]);

    /* switch mode */
    alloc = alloc_post;
    alloc0 = alloc0_post;
    delete = delete_post;
    delete0 = delete0_post;
}

//

#include <textos/boot.h>
#include <textos/uefi.h>

void __kpg_dump(kpgs_t *kpg)
{
    for (int i = 0;; i++, kpg++)
    {
        if (!kpg->va)
            break;
        DEBUGK(K_INIT, "[#%2d] kpg %p -> %p | %d\n", i, kpg->phy, kpg->vrt, kpg->msiz);
    }
}

void __pmm_pre()
{
    DEBUGK(K_INIT, "early-init physical memory!\n");
    if (bmode_get() == BOOT_EFI)
    {
        bconfig_t *b = binfo_get();
        mconfig_t *m = &b->memory;
        __kpg_dump((kpgs_t *)m->kpgs);

        free_t *n = &_free;
        mapinfo_t *info = m->map;
        EFI_MEMORY_DESCRIPTOR *desc = info->maps;
        for (int i = 0; i < info->mapcount; i++, desc = OFFSET(desc, info->descsiz))
        {
            DEBUGK(K_INIT, "[#%02d] %p | %p | %s\n", i, desc->PhysicalStart, desc->NumberOfPages,
                get_uefi_mtstr(desc->Type));
            page_total += desc->NumberOfPages;

            if (!desc->NumberOfPages || !desc->PhysicalStart)
                continue;
            if (desc->Type == EfiBootServicesData ||
                desc->Type == EfiBootServicesCode)
                desc->Type = EfiConventionalMemory;
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
    }
    if (bmode_get() == BOOT_MB1)
    {
        free_t *n = &_free;
        multiboot_info_t *b = binfo_get();
        multiboot_memory_map_t *m = (void *)(uintptr_t)b->mmap_addr;
        int mapcount = b->mmap_length / sizeof(multiboot_memory_map_t);
        for (int i = 0 ; i < mapcount ; i++, m++)
        {
            DEBUGK(K_INIT, "[#%02d] %p | %p | %d\n", i, m->addr, m->len / PAGE_SIZE, m->type);
            page_total += m->len / PAGE_SIZE;
            if (!m->len || !m->addr)
                continue;
            if (m->type != MULTIBOOT_MEMORY_AVAILABLE)
                continue;
            page_free += m->len / PAGE_SIZE;

            n->next = (free_t *)m->addr;
            n = n->next;
            n->addr = (u64)n;
            n->pages = m->len / PAGE_SIZE;
        }
    }

    alloc = alloc_early;
    alloc0 = alloc_early;
    delete = delete_early;
    delete0 = delete_early;
}

void __pmm_pre_mb()
{
}
