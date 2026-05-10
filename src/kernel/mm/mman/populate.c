#include <textos/task.h>
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
    // either explicitly requested via MAP_POPULATE, or forced by task flag.
    // when debugging user apps under qemu, executing unmapped code will trap
    // into kernel, which disturbs the debugging experience. :(
    if ((vm->flgs & MAP_POPULATE) ||
        ((vm->prot & PROT_EXEC) && task_current()->dbg_byemu)) {
        mmap_populate(vm);
    }
}
