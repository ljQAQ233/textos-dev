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
        if (fd < MAX_FILE)
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

    void *ret = MRET(-EINVAL);
    if (!vm.num)
        goto done;
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
