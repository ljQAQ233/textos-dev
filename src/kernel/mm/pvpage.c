/* make temporary pages mapping to virtual mem space */

#include <textos/mm.h>
#include <textos/assert.h>
#include <textos/klib/bitmap.h>

static bitmap_t map;

int make_pvpage(addr_t phy, void *res)
{
    size_t idx = bitmap_find(&map);
    if (idx == -1)
        return -1;

    addr_t vrt = __kern_pvpg_base + PAGE_SIZ * idx;
    vmap_map(phy, vrt, 1, PE_P | PE_RW, MAP_4K);
    *((addr_t *)res) = vrt;
    return 0;
}

void break_pvpage(void *page)
{
    size_t idx = ((addr_t)page - __kern_pvpg_base) / PAGE_SIZ;
    bitmap_reset(&map, idx);
}

void pvpage_init()
{
    #define siz \
        (__kern_pvpg_max - __kern_pvpg_base) / PAGE_SIZ

    static mpunit buf[siz / mpemsz];
    bitmap_init(&map, buf, siz);
}
