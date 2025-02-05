#include <textos/mm.h>
#include <textos/mm/map.h>
#include <textos/assert.h>

/* Check if the Vrt is a canonical format
   vrt addr and adjust it if it is invalid

   @retval  int   The state */
int vmm_canadjust (u64 *vrt)
{
    if ((*vrt >> 47) & 1) {
        if (*vrt >> 48 == 0xFFFF)
            return true;

        *vrt |= 0xffff8ull << 44;
        return false;
    }

    return true;
}

void *vmm_phyauto (u64 vrt, size_t num, u16 flgs)
{
    ASSERTK (!(vrt & PAGE_MASK) && vrt != 0); // 确保它不是 NULL 并且是一页开始的地方

    if (!vmm_canadjust (&vrt))
        DEBUGK (K_MM, "the addr is not a canonical addr, adjust it - %p\n",vrt);

    void *page = pmm_allocpages (num);

    vmap_map ((u64)page, vrt, num, flgs, MAP_4K);

    return (void *)vrt;
}

static u64 _idx;

void *vmm_allocvrt(size_t num)
{
    void *page = (void *)__kern_vmm_pages + PAGE_SIZ * _idx;
    _idx += num;
    return page;
}

/* TODO: complete it */
void *vmm_allocpages (size_t num, u16 flgs)
{
    void *page = vmm_allocvrt(num);
    vmm_phyauto ((u64)page, num, flgs);

    return page;
}

void vmm_allocpv(size_t num, addr_t *va, addr_t *pa)
{
    addr_t vp = (addr_t)vmm_allocvrt(num);
    addr_t pp = (addr_t)pmm_allocpages(num);
    vmap_map(pp, vp, num, PE_P | PE_RW, MAP_4K);
    *va = vp;
    *pa = pp;
}

/* TODO: Design memory space and set up VMM */
