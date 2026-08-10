#include <textos/boot.h>

#include "common.h"

static __bcode void flush_tlb(void);
static __bdata getpage_t *boot_getpage;
static __bdata addr_t boot_offset;

#define boot_reloc(x) (fq(x) + boot_offset)

static __bcode int mappage(addr_t pa, addr_t va, int flag)
{
    int pml4_index = (va >> 39) & 0x1ff;
    int pdp_index = (va >> 30) & 0x1ff;
    int pd_index = (va >> 21) & 0x1ff;
    int pt_index = (va >> 12) & 0x1ff;
    addr_t *bootpgt = (addr_t *)boot_reloc(__bootpgt);
    addr_t *pml4_entry = bootpgt + pml4_index;
    if (!(*pml4_entry & 1))
    {
        addr_t new_pdp = (addr_t)boot_getpage();
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
        addr_t new_pd = (addr_t)boot_getpage();
        if (!new_pd)
            return -1;
        for (int i = 0; i < 512; i++)
            ((addr_t *)new_pd)[i] = 0;
        *pdp_entry = new_pd | 0x03;
    }

    addr_t *pd_entry = (addr_t *)(*pdp_entry & ~0xfff) + pd_index;
    if (!(*pd_entry & 1))
    {
        addr_t new_pt = (addr_t)boot_getpage();
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

__bcode void __common64(long magic, long info, getpage_t *getpage, addr_t offset)
{
    boot_getpage = getpage;
    boot_offset = offset;

    addr_t maps[3][3] = {
        { fq(__kx_start), fq(__kx_end), 1 },
        { fq(__kr_start), fq(__kr_end), 2 },
        { fq(__kw_start), fq(__kw_end), 3 },
    };
    for (int i = 0 ; i < 3 ; i++)
    {
        addr_t vstart = align_dn(maps[i][0], PAGE_SIZE);
        addr_t vend = align_up(maps[i][1], PAGE_SIZE);
        addr_t pstart = vstart - vbase + offset;
        size_t size = vend - vstart;
        maprange(pstart, vstart, size, maps[i][2]);
    }
    flush_tlb();

    void (*init)(long, long, long, long);
    init = (void *)fq(kernel_init);
    init(magic, info, 0, 0);
}
