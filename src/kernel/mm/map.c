#include <cpu.h>
#include <textos/debug.h>
#include <textos/assert.h>
#include <textos/mm.h>
#include <textos/mm/map.h>

#include <string.h>

static u64 *pml4;

/* Value */
#define PE_V_FLAGS(entry) (((u64)entry) & 0x7FF)
#define PE_V_ADDR(entry) (((u64)entry) & ~0x7FF)

/* Set */
#define PE_S_FLAGS(flgs) (u64)(flgs & 0x7FF)
#define PE_S_ADDR(addr) (u64)(((u64)addr & ~0x7FF))

#define IDX(addr, level) (((u64)addr >> ((int)(level - 1) * 9 + 12)) & 0x1FF)

enum level {
    L_PML4 = 4,
    L_PDPT = 3,
    L_PD   = 2,
    L_PT   = 1,
    L_PG   = 0,
};

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

        memset (new, 0, PAGE_SIZ);
        tab[idx] = PE_S_ADDR(new) | PE_RW | PE_P;
    }
    tab = (u64 *)PE_V_ADDR(tab[idx]);

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
    else if (md == PDPTE_1G)
        return SIZE_1GB;

    return SIZE_4KB;
}

void vmap_map (u64 phy, u64 vrt, size_t num, u16 flgs, int mode)
{
    DEBUGK (K_MM, "try to map %p -> %p - %llu,%x,%d\n", phy, vrt, num, flgs, mode);

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
    pml4 = (u64 *)PE_V_ADDR (read_cr3());
    DEBUGK(K_INIT, "kpgt - pml4 = %p\n", pml4);
}

