#include <textos/fs.h>
#include <textos/task.h>
#include <textos/mm/mman.h>

/*
 * map regular files
 */
void *vfs_generic_mmap(node_t *n, vm_region_t *v)
{
    vm_area_t *vma = vmm_new_vma(0);
    vma->s = v->va;
    vma->t = v->va + PAGE_SIZE * v->num;
    vma->flgs = v->flgs;
    vma->prot = v->prot;
    vma->label = MAPL_FILE;
    vma->obj.node = n;
    vma->obj.foff = v->foff;
    vmm_sp_regst(task_current()->vsp, vma);
    return MRET(vma->s);
}
