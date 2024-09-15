#include <cpu.h>
#include <textos/debug.h>
#include <textos/assert.h>
#include <textos/mm.h>
#include <textos/mm/map.h>

#include <string.h>

static u64 *pml4;
static u64 cr3;

/* Value */
#define PE_V_FLAGS(entry) (((u64)entry) & 0x7FF)
#define PE_V_ADDR(entry) (((u64)entry) & ~0x7FF)

/* Set */
#define PE_S_FLAGS(flgs) (u64)(flgs & 0x7FF)
#define PE_S_ADDR(addr) (u64)(((u64)addr & ~0x7FF))

#define IDX(addr, level) (((u64)addr >> ((int)(level - 1) * 9 + 12)) & 0x1FF)

#define R_IDX 511ULL

#define VRT(iPML4, iPDPT, iPD, iPT) \
    ((u64)(((u64)iPML4 > 255 ? 0xFFFFULL : 0) << 48 \
    | ((u64)iPML4 & 0x1FF) << 39 \
    | ((u64)iPDPT & 0x1FF) << 30 \
    | ((u64)iPD & 0x1FF) << 21   \
    | ((u64)iPT & 0x1FF) << 12))

enum level {
    L_PML4 = 4,
    L_PDPT = 3,
    L_PD   = 2,
    L_PT   = 1,
    L_PG   = 0,
};

/* Locate entry or others by virtual address */
#define PML4E_IDX(addr) (((u64)addr >> 39) & 0x1FF)
#define PDPTE_IDX(addr) (((u64)addr >> 30) & 0x1FF)
#define PDE_IDX(addr)   (((u64)addr >> 21) & 0x1FF)
#define PTE_IDX(addr)   (((u64)addr >> 12) & 0x1FF)

#include <textos/panic.h>

/* Get vrt addr of pgt by addr & level */
static inline u64*
_vrt_entryget (u64 addr, int level)
{
    if (level == L_PML4)
        return (u64 *)VRT(R_IDX, R_IDX, R_IDX, R_IDX);
    else if (level == L_PDPT)
        return (u64 *)VRT(R_IDX, R_IDX, R_IDX, PML4E_IDX(addr));
    else if (level == L_PD)
        return (u64 *)VRT(R_IDX, R_IDX, PML4E_IDX(addr), PDPTE_IDX(addr));
    else if (level == L_PT)
        return (u64 *)VRT(R_IDX, PML4E_IDX(addr), PDPTE_IDX(addr), PDE_IDX(addr));

    PANIC ("Unable to get the vrt addr of the pgt by addr & level - %p, %d\n", addr, level);

    return NULL;
}

static void
_map_walk (u64 phy, u64 vrt, u16 flgs, u64 *tab, int level, int mode)
{
    u64 idx = IDX(vrt, level);

    if (level == mode) {
        tab[idx] = PE_S_ADDR(phy) | PE_S_FLAGS(flgs);
        return;
    }
    else if (!(tab[idx] & PE_P))
    {
        void *new = pmm_allocpages(1);
        ASSERTK (new != NULL);

        tab[idx] = PE_S_ADDR(new) | PE_RW | PE_P;
        __asm__ volatile ("invlpg (%0)" : : "r"(tab) : "memory");
        new = _vrt_entryget (vrt, level - 1);
        memset (new, 0, PAGE_SIZ);
    }
    
    if (flgs & PE_US)
        tab[idx] |= PE_US;

    tab = (u64 *)_vrt_entryget (vrt, level - 1);
    _map_walk (phy, vrt, flgs, tab, --level, mode);
}

static inline u16
parse_flags (int md)
{
    if (md == MAP_2M)
        return PDE_2M;
    else if (md == MAP_1G)
        return PDPTE_1G;

    return 0;
}

static inline u32
_pagesiz (int md)
{
    if (md == MAP_2M)
        return SIZE_2MB;
    else if (md == MAP_1G)
        return SIZE_1GB;

    return SIZE_4KB;
}

void vmap_map (u64 phy, u64 vrt, size_t num, u16 flgs, int mode)
{
    DEBUGK(K_MM, "try to map %p -> %p - %llu,%x,%d\n", phy, vrt, num, flgs, mode);

    u64 pagesiz = _pagesiz (mode);
    while (num--) {
        _map_walk (phy, vrt, flgs | parse_flags (mode), pml4, L_PML4, mode);
        phy += pagesiz;
        vrt += pagesiz;
    }

    __asm__ volatile ("invlpg (%0)" : : "r"(vrt) : "memory");
}

void vmap_init ()
{
    cr3 = read_cr3();
    pml4 = (u64 *)cr3;
    pml4[R_IDX] = PE_S_ADDR(cr3) | PE_RW | PE_P;
    
    // set pgt into vrt mode
    pml4 = _vrt_entryget (0, L_PML4);
    write_cr3(cr3);
    DEBUGK(K_INIT, "kpgt - pml4 = %p\n", pml4);
}

extern void __uefi_tovmm();
extern void __pmm_tovmm();
extern void __apic_tovmm();
extern void __video_tovmm();

void vmap_initvm ()
{
    vmap_map(0, __kern_phy_offet, __kern_phy_mapsz / _pagesiz(MAP_1G), PE_P | PE_RW, MAP_1G);

    // As callback functions
    __uefi_tovmm();
    __pmm_tovmm();
    __apic_tovmm();
    __video_tovmm();
    
    for (int i = 0; i < 256 ;i++)
        if (i != R_IDX)
            pml4[i] &= ~PE_P;
    DEBUGK(K_INIT, "remap completed!\n");
}

addr_t get_kpgt()
{
    return (addr_t)(pml4);
}

addr_t get_kppgt()
{
    return cr3;
}

#include <textos/mm/pvpage.h>

static addr_t _copy_pgtd(addr_t pg, int level)
{
    addr_t npg;
    u64 *vpg, *vnpg;
    npg = (addr_t)pmm_allocpages(1);
    
    while (make_pvpage((addr_t)pg, &vpg) < 0);
    while (make_pvpage((addr_t)npg, &vnpg) < 0);
    memcpy(vnpg, vpg, PAGE_SIZ);

    if (level == L_PG)
        goto done;

    int limit = level == L_PML4 ? 256 : 512;
    for (int i = 0 ; i < limit ; i++)
    {
        if (vnpg[i])
        {
            addr_t paddr = _copy_pgtd(PE_V_ADDR(vnpg[i]), level - 1);
            vnpg[i] = PE_V_FLAGS(vnpg[i]) | PE_V_ADDR(paddr);
        }
    }

done:
    break_pvpage(vpg);
    break_pvpage(vnpg);
    return npg;
}

/* deep-fork */
addr_t copy_pgtd(addr_t ppgt)
{
    return (addr_t)_copy_pgtd(ppgt, L_PML4);
}
