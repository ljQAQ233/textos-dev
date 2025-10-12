#include <textos/mm.h>
#include <textos/mm/map.h>
#include <textos/assert.h>

int vmm_canadjust(addr_t *vrt)
{
    if ((*vrt >> 47) & 1)
    {
        if (*vrt >> 48 == 0xFFFF)
            return true;
        *vrt |= 0xffff8ull << 44;
        return false;
    }
    return true;
}

void *vmm_phyauto(addr_t vrt, size_t num, int flgs)
{
    ASSERTK(!(vrt & PAGE_MASK) && vrt != 0); // 确保它不是 NULL 并且是一页开始的地方
    if (!vmm_canadjust(&vrt))
        DEBUGK(K_MM, "the addr is not a canonical addr, adjust it - %p\n", vrt);
    size_t unit = num;
    while (num) {
        addr_t page = pmm_allocpages(num);
        if (!page) {
            unit /= 2;
            // no free pages
            ASSERTK(unit != 0);
            continue;
        }
        vmap_map(page, vrt, num, flgs);
        vrt += unit * PAGE_SIZE;
        num -= unit;
    }
    return (void *)vrt;
}

void *vmm_allocvrt(size_t num)
{
    static u64 idx;
    void *page = (void *)__kern_vmm_pages + PAGE_SIZ * idx;
    idx += num;
    return page;
}

/* TODO: complete it */
void *vmm_allocpages(size_t num, int flgs)
{
    void *page = vmm_allocvrt(num);
    vmm_phyauto((addr_t)page, num, flgs);
    return page;
}

void vmm_allocpv(size_t num, addr_t *va, addr_t *pa)
{
    addr_t vp = (addr_t)vmm_allocvrt(num);
    addr_t pp = (addr_t)pmm_allocpages(num);
    vmap_map(pp, vp, num, PE_P | PE_RW);
    *va = vp;
    *pa = pp;
}

/* TODO: Design memory space and set up VMM */
