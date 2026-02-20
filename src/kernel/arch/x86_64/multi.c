#include <textos/boot.h>
#include <textos/user/elf.h>

static __bdata void *mmap;
static __bdata size_t mmap_max;

// fetch qword
#define fq(x) ({ \
    addr_t __res;                 \
    __asm__ ("movabs $" #x ", %0" \
        : "=r"(__res)); __res;    \
})

// physical address of a qword var
#define pq(x) (fq(x) - __vbase)

#define lbase fq(__lbase)
#define vbase fq(__vbase)
#define lend  fq(__lend)

#define align_up(x, y) ((y) * ((x + y - 1) / y))
#define align_dn(x, y) ((y) * (x / y))

static __bcode void reskern()
{
    addr_t kern_start = lbase;
    addr_t kern_end   = align_up(lend, PAGE_SIZE);

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
            multiboot_memory_map_t *newmap = &((multiboot_memory_map_t *)mmap)[0];
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
static __bcode void flush_tlb(void);

static __bcode int mappage(addr_t pa, addr_t va, int flag)
{
    int pml4_index = (va >> 39) & 0x1ff;
    int pdp_index = (va >> 30) & 0x1ff;
    int pd_index = (va >> 21) & 0x1ff;
    int pt_index = (va >> 12) & 0x1ff;
    addr_t *bootpgt = (addr_t *)fq(__bootpgt);
    addr_t *pml4_entry = bootpgt + pml4_index;
    if (!(*pml4_entry & 1))
    {
        addr_t new_pdp = (addr_t)getpage();
        if (!new_pdp)
            return -1;
        for (int i = 0; i < 512; i++)
            ((addr_t *)new_pdp)[i] = 0;
        *pml4_entry = new_pdp | 0x03;
        flush_tlb();
    }

    addr_t *pdp_entry = (addr_t *)(*pml4_entry & ~0xfff) + pdp_index;
    if (!(*pdp_entry & 1))
    {
        addr_t new_pd = (addr_t)getpage();
        if (!new_pd)
            return -1;
        for (int i = 0; i < 512; i++)
            ((addr_t *)new_pd)[i] = 0;
        *pdp_entry = new_pd | 0x03;
    }

    addr_t *pd_entry = (addr_t *)(*pdp_entry & ~0xfff) + pd_index;
    if (!(*pd_entry & 1))
    {
        addr_t new_pt = (addr_t)getpage();
        if (!new_pt)
            return -1;
        for (int i = 0; i < 512; i++)
            ((addr_t *)new_pt)[i] = 0;
        *pd_entry = new_pt | 0x03;
    }

    addr_t *pt_entry = (addr_t *)(*pd_entry & ~0xfff) + pt_index;
    *pt_entry = pa | flag | 0x01;
    return 0;
}

static __bcode int maprange(addr_t pa, addr_t va, size_t size, int flag)
{
    for (size_t off = 0; off < size; off += PAGE_SIZE)
        if (mappage(pa + off, va + off, flag) < 0)
            return -1;
    return 0;
}

static __bcode void flush_tlb(void)
{
    addr_t cr3;
    __asm__ __volatile__(
        "mov %%cr3, %0\n"
        "mov %0, %%cr3\n"
        : "=r"(cr3)
        :
        : "memory"
    );
}

__bcode void __multi64(long magic, long info)
{
    multiboot_info_t *mi = (void *)info;
    mmap = (void *)(long)mi->mmap_addr;
    mmap_max = mi->mmap_length / sizeof(multiboot_memory_map_t);
    reskern();

    addr_t maps[3][3] = {
        { fq(__kx_start), fq(__kx_end), 1 },
        { fq(__kr_start), fq(__kr_end), 2 },
        { fq(__kw_start), fq(__kw_end), 3 },
    };
    for (int i = 0 ; i < 3 ; i++)
    {
        addr_t vstart = align_dn(maps[i][0], PAGE_SIZE);
        addr_t vend = align_up(maps[i][1], PAGE_SIZE);
        addr_t pstart = vstart - vbase;
        size_t size = vend - vstart;
        maprange(pstart, vstart, size, maps[i][2]);
    }
    flush_tlb();

    void (*init)(long, long, long, long);
    init = (void *)fq(kernel_init);
    init(magic, info, 0, 0);
}
