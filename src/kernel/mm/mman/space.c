#include <textos/mm/mman.h>
#include <textos/mm/heap.h>

vm_space_t *mm_new_space()
{
    vm_space_t *sp = malloc(sizeof(vm_space_t));
    list_init(&sp->list);
    sp->tree = RBTREE_INIT();
    return sp;
}
