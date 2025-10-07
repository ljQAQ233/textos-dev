#include <textos/mm/mman.h>
#include <textos/mm/heap.h>

vm_space_t *mm_new_space(vm_space_t *old)
{
    vm_space_t *sp = malloc(sizeof(vm_space_t));
    list_init(&sp->list);
    sp->tree = RBTREE_INIT();
    if (old)
    {
        list_t *ptr;
        LIST_FOREACH(ptr, &old->list)
        {
            vm_area_t *o = CR(ptr, vm_area_t, list);
            vm_area_t *a = mmap_new_vma(o);
            mmap_regst(sp, a);
        }
    }
    return sp;
}
