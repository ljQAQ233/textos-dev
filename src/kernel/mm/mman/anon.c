#include <textos/task.h>
#include <textos/mm/map.h>
#include <textos/mm/vmm.h>
#include <textos/mm/mman.h>

void *mmap_anon(vm_region_t *vm)
{
    int mapflg = 0;
    mapflg |= PE_P | PE_US;
    mapflg |= mapprot(vm->prot);
    vm_area_t *vma = vmm_new_vma(0);
    vma->s = vm->va;
    vma->t = vm->va + vm->num * PAGE_SIZE;
    vma->flgs = vm->flgs;
    vma->prot = vm->prot;
    vmm_sp_regst(task_current()->vsp, vma);
    return MRET(vma->s);
}
