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

__SYSCALL_DEFINE6(void *, mmap, void *, addr, size_t, len, int, prot, int, flgs, int, fd, size_t, off)
{
    if ((flgs & MAP_TYPE) == 0)
        return MRET(-EINVAL);

    node_t *node = NULL;
    if (0 <= fd)
    {
        if (fd >= MAX_FILE)
            return MRET(-EBADF);
        file_t *file = task_current()->files[fd];
        if (file)
            node = file->node;
    }

    vm_region_t vm = {
        .va = (addr_t)addr,
        .num = DIV_ROUND_UP(len, PAGE_SIZ),
        .prot = prot,
        .flgs = flgs,
        .foff = off,
        .fnode = node,
        .ppgs = NULL,
    };

    // do check
    void *ret = MRET(-EINVAL);
    if (!vm.num)
        goto done;
    if (vm.va & PAGE_MASK)
        goto done;
    if (vm.foff & PAGE_MASK)
        goto done;

    // select address first
    if (~vm.flgs & MAP_FIXED)
        vm.va = vmm_fitaddr(vm.va, vm.num);
    else
        munmap(addr, len);

    // choose mmap handler
    if (flgs & MAP_ANON)
    {
        if (fd != -1 || off != 0)
            goto done;
        ret = mmap_anon(&vm);
    }
    else
    {
        ret = mmap_file(&vm);
        DEBUGK(K_LOGK, "mmap file %s -> %p\n", ((node_t *)vm.fnode)->name, ret);
    }

done:
    return ret;
}

__SYSCALL_DEFINE3(int, mprotect, void *, addr, size_t, len, int, prot)
{
    return -EINVAL;
}

static inline int covered(vm_area_t *vma, addr_t s, addr_t t)
{
    return MAX(vma->s, s) < MIN(vma->t, t);
}

static inline void unmap(vm_area_t *vma, addr_t va)
{
    addr_t pa = vmap_query(va);
    vm_ppgrec_t *rec = vmm_ppg_query(vma, pa);
    if (rec != NULL)
        vmm_ppg_unref(rec);
    vmap_unmap(va, 1);
}

#define A(l) CR((l), vm_area_t, list)

__SYSCALL_DEFINE2(int, munmap, void *, addr, size_t, len)
{
    // do check:
    //   - make sure `addr` and `len` aligned with PAGE_SIZE
    //   - len is valid (not zero or tooooo big!)
    addr_t ask_start = (addr_t)addr;
    addr_t ask_end = ask_start + len;
    if ((ask_start | ask_end) & PAGE_MASK)
        return -EINVAL;
    if (ask_start >= ask_end)
        return -EINVAL;

    // find the first vma whose start address is greater than `addr`.
    // that vma itself cannot cover `addr`, but the one right before it might.
    // if no such vma exists, the last vma in the list becomes the candidate.
    vm_space_t *sp = task_current()->vsp;
    vm_area_t *a = vmm_sp_upperbound(sp, ask_start);
    a = a ? A(a->list.prev) : A(sp->list.prev);
    a = covered(a, ask_start, ask_end) ? a : A(a->list.next);
    for (list_t *ptr = &a->list ; ptr != &sp->list ; ptr = ptr->next)
    {
        if (!covered(a = A(ptr), ask_start, ask_end))
            break;
        addr_t cov_start = MAX(a->s, ask_start);
        addr_t cov_end = MIN(a->t, ask_end);
        for (addr_t cur = cov_start ; cur < cov_end ; cur += PAGE_SIZE)
            unmap(a, cur);
        // the small target area to be freed is covered by vma,
        // then split it into 2 parts
        if (a->s < ask_start && ask_end < a->t)
        {
            vm_area_t *na = vmm_new_vma(a);
            a->t = ask_start;
            na->s = ask_end;
            vmm_sp_regst(sp, na);
        }
        // else vma is covered by [ask_start, ask_end)
        // [cov_start, cov_end) covers the whole vma
        else if (cov_start == a->s && a->t == cov_end)
            vmm_sp_unreg(sp, a);
        // left-aligned
        else if (a->s == cov_start)
            a->s = cov_end;
        // right-aligned
        else if (a->t == cov_end)
            a->t = cov_start;
    }
    return 0;
}
