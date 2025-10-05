#include <cpu.h>
#include <textos/assert.h>
#include <textos/mm.h>
#include <textos/mm/map.h>

#include <string.h>

static u64 *pml4;
static u64 cr3;

#define FLAG(entry) (((u64)entry) & 0x7FF)
#define ADDR(entry) (((u64)entry) & ~0x7FF)

#define VRT(iPML4, iPDPT, iPD, iPT) \
    ((u64)(((u64)iPML4 > 255 ? 0xFFFFULL : 0) << 48 \
    | ((u64)iPML4 & 0x1FF) << 39 \
    | ((u64)iPDPT & 0x1FF) << 30 \
    | ((u64)iPD & 0x1FF) << 21   \
    | ((u64)iPT & 0x1FF) << 12))

/* * level means where it locates currently */
#define L_PML4 3
#define L_PDPT 2
#define L_PD 1
#define L_PT 0
#define L_PG -1

/* Locate entry or others by virtual address */
#define IDX(addr, level) ((u64)addr >> (level * 9 + 12) & 0x1FF)
#define PML4E_IDX(addr) IDX(addr, L_PML4)
#define PDPTE_IDX(addr) IDX(addr, L_PDPT)
#define PDE_IDX(addr) IDX(addr, L_PD)
#define PTE_IDX(addr) IDX(addr, L_PT)

/* Recursive pagetable idx */
#define R_IDX 511ULL

#include <textos/panic.h>

/* Get vrt addr of pgt by addr & level */
static inline u64 *entryget(u64 addr, int level)
{
    if (level == L_PML4)
        return (u64 *)VRT(R_IDX, R_IDX, R_IDX, R_IDX);
    else if (level == L_PDPT)
        return (u64 *)VRT(R_IDX, R_IDX, R_IDX, PML4E_IDX(addr));
    else if (level == L_PD)
        return (u64 *)VRT(R_IDX, R_IDX, PML4E_IDX(addr), PDPTE_IDX(addr));
    else if (level == L_PT)
        return (u64 *)VRT(R_IDX, PML4E_IDX(addr), PDPTE_IDX(addr), PDE_IDX(addr));
    return NULL;
}

static inline void invlpg(u64 vrt)
{
    __asm__ volatile("invlpg (%0)" : : "r"(vrt) : "memory");
}

static void map_walk(u64 phy, u64 vrt, u16 flgs, int lv)
{
    u64 *tab = entryget(vrt, lv);
    u64 idx = IDX(vrt, lv);
    if (lv == 0)
    {
        tab[idx] = ADDR(phy) | FLAG(flgs);
        invlpg(vrt);
        return;
    }
    if (!(tab[idx] & PE_P))
    {
        void *new = pmm_allocpages(1);
        ASSERTK(new != NULL);

        tab[idx] = ADDR(new) | PE_RW | PE_P;
        invlpg((u64)tab);
        new = entryget(vrt, lv - 1);
        memset(new, 0, PAGE_SIZ);
    }

    if (flgs & PE_US)
        tab[idx] |= PE_US;

    map_walk(phy, vrt, flgs, lv - 1);
}

static u64 qry_walk(u64 vrt, int lv)
{
    int idx = IDX(vrt, lv);
    u64 entry = entryget(vrt, lv)[idx];
    if (!(entry & PE_P))
        return 0;
    if (!lv)
        return ADDR(entry) | vrt & ((1 << (lv * 9 + 12)) - 1);
    return qry_walk(vrt, lv - 1);
}

void vmap_map(addr_t phy, addr_t vrt, size_t num, int flgs)
{
    size_t pgsz = PAGE_SIZE;
    while (num--)
    {
        map_walk(phy, vrt, flgs, L_PML4);
        phy += pgsz, vrt += pgsz;
    }
}

addr_t vmap_query(addr_t vrt)
{
    return qry_walk(vrt, L_PML4);
}

void vmap_init()
{
    cr3 = read_cr3();
    pml4 = (u64 *)cr3;
    pml4[R_IDX] = ADDR(cr3) | PE_RW | PE_P;

    // set pgt into vrt mode
    pml4 = entryget(0, L_PML4);
    write_cr3(cr3);
    DEBUGK(K_INIT, "kpgt - pml4 = %p\n", pml4);
}

extern void __uefi_tovmm();
extern void __pmm_tovmm();
extern void __acpi_tovmm();
extern void __apic_tovmm();
extern void __video_tovmm();

void vmap_initvm()
{
    // As callback functions
    __uefi_tovmm();
    __pmm_tovmm();
    __acpi_tovmm();
    __apic_tovmm();
    __video_tovmm();

    for (int i = 0; i < 256; i++)
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

static u64 _copy_pgtd(u64 pg, int lv)
{
    u64 npg;
    u64 *vpg, *vnpg;
    npg = (u64)pmm_allocpages(1);

    while (make_pvpage((u64)pg, &vpg) < 0)
        ;
    while (make_pvpage((u64)npg, &vnpg) < 0)
        ;
    memcpy(vnpg, vpg, PAGE_SIZ);

    if (lv == L_PG)
        goto done;

    int limit = lv == L_PML4 ? 256 : 512;
    for (int i = 0; i < limit; i++)
    {
        if (vnpg[i])
        {
            u64 paddr = _copy_pgtd(ADDR(vnpg[i]), lv - 1);
            vnpg[i] = FLAG(vnpg[i]) | ADDR(paddr);
        }
    }

    if (lv == L_PML4)
        vnpg[R_IDX] = ADDR(npg) | PE_RW | PE_P;

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
