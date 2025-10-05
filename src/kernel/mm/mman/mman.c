/*
 * user mmap operations
 * comment written by maouai233 and some from POSIX manual (`man mmap`)
 */

#include <textos/mm.h>
#include <textos/mm/map.h>
#include <textos/mm/pmm.h>
#include <textos/mm/vmm.h>
#include <textos/mm/mman.h>
#include <textos/file.h>
#include <textos/task.h>
#include <textos/errno.h>
#include <textos/syscall.h>

void *mmap_file(vm_region_t *vm)
{
    file_t *file = vm->file;
    return file->node->opts->mmap(file->node, vm);
}

void *mmap_anon(vm_region_t *vm)
{
    addr_t vaddr;
    int mapflg = 0;

    mapflg |= PE_US;
    if (vm->prot & PROT_READ)
        mapflg |= PE_P;
    if (vm->prot & PROT_WRITE)
        mapflg |= PE_P | PE_RW;
    if (~vm->prot & PROT_EXEC)
        mapflg |= PE_NX;
    if (vm->flgs & MAP_FIXED) {
        vaddr = vm->va;
    } else {
        task_t *tsk = task_current();
        vaddr = tsk->mmap;
        tsk->mmap += vm->num * PAGE_SIZ;
    }
    return vmm_phyauto(vaddr, vm->num, mapflg);
}

__SYSCALL_DEFINE6(void *, mmap, void *, addr, size_t, len, int, prot, int, flgs, int, fd, size_t, off)
{
    void *ret = MAP_FAILED;
    file_t *file = NULL;

    if ((flgs & MAP_TYPE) == 0)
        goto done;

    // TODO: replace it
    if (0 <= fd && fd < MAX_FILE)
        file = task_current()->files[fd];

    vm_region_t vm = {
        .va = (addr_t)addr,
        .num = (size_t)(DIV_ROUND_UP(len, PAGE_SIZ)),
        .prot = prot,
        .flgs = flgs,
        .foff = off,
        .file = file,
        .ppgs = NULL,
    };

    if (flgs & MAP_ANON) {
        if (fd != -1 || off != 0)
            goto done;
        ret = mmap_anon(&vm);
    } else {
        ret = mmap_file(&vm);
    }

done:
    return ret;
}

__SYSCALL_DEFINE3(int, mprotect, void *, addr, size_t, len, int, prot)
{
    return -EINVAL;
}

__SYSCALL_DEFINE2(int, munmap, void *, addr, size_t, len)
{
    return -EINVAL;
}
