#include <textos/mm/mman.h>

void *mmap_file(vm_region_t *vm)
{
    node_t *node = vm->fnode;
    struct fs_openctx ctx = {0};
    void *ret = node->opts->mmap(node, vm, &ctx);
    __mmap_populate_cond(vm);
    return ret;
}
