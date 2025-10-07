#include <textos/task.h>
#include <textos/mm/map.h>
#include <textos/mm/vmm.h>
#include <textos/mm/mman.h>

void *mmap_anon(vm_region_t *vm)
{
    addr_t vaddr;
    int mapflg = 0;

    mapflg |= PE_P | PE_US;
    mapflg |= mapprot(vm->prot);
    if (vm->flgs & MAP_FIXED) {
        vaddr = vm->va;
    } else {
        task_t *tsk = task_current();
        vaddr = tsk->mmap;
        tsk->mmap += vm->num * PAGE_SIZ;
    }
    vm_area_t *vma = mmap_new_vma(0);
    vma->s = (addr_t)vaddr;
    vma->t = (addr_t)vaddr + vm->num * PAGE_SIZE;
    vma->flgs = vm->flgs;
    vma->prot = vm->prot;
    mmap_regst(task_current()->vsp, vma);
    return MRET(vma->s);
}
