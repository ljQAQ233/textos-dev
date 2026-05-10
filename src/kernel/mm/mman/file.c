#include <textos/mm/mman.h>

void *mmap_file(vm_region_t *vm)
{
    node_t *node = vm->fnode;
    void *ret = node->opts->mmap(node, vm);
    __mmap_populate_cond(vm);
    return ret;
}
