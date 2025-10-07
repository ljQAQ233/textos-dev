#include <textos/mm/mman.h>

void *mmap_file(vm_region_t *vm)
{
    node_t *node = vm->fnode;
    return node->opts->mmap(node, vm);
}
