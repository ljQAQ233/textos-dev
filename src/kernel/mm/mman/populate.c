#include <textos/mm/mman.h>

void mmap_populate(vm_region_t *vm)
{
    volatile char *addr = (void *)vm->va;
    for (int i = 0; i < vm->num; i++) {
        (void)*addr;
        addr += PAGE_SIZE;
    }
}

/**
 * @brief prefault a vm_region_t if needed
 */
void __mmap_populate_cond(vm_region_t *vm)
{
    if (vm->flgs & MAP_POPULATE) mmap_populate(vm);
}
