/*
 * frame buffer device
 */
#include <textos/dev.h>
#include <textos/task.h>
#include <textos/errno.h>
#include <textos/video.h>
#include <textos/mm/map.h>
#include <textos/mm/mman.h>

static void *fbdev_mmap(devst_t *fb, vm_region_t *vm)
{
    addr_t vaddr;
    addr_t paddr;
    size_t fblen;
    size_t pgnum;
    int mapflg = 0;

    mapflg |= PE_US;
    if (vm->prot & PROT_READ)
        mapflg |= PE_P;
    if (vm->prot & PROT_WRITE)
        mapflg |= PE_P | PE_RW;
    if (vm->prot & PROT_EXEC)
        return MRET(-EINVAL);

    if (vm->flgs & MAP_FIXED) {
        vaddr = vm->va;
    } else {
        task_t *tsk = task_current();
        vaddr = tsk->mmap;
        tsk->mmap += vm->num * PAGE_SIZ;
    }
    screen_info5(NULL, &paddr, &fblen, NULL, NULL);
    pgnum = DIV_ROUND_UP(fblen, PAGE_SIZ); // 多此一举

    vmap_map(paddr, vaddr, pgnum, mapflg);
    return MRET(vaddr);
}

void fbdev_init()
{
    devst_t *fb = dev_new();
    fb->name = "fb0";
    fb->mmap = fbdev_mmap;
    fb->type = DEV_CHAR;
    fb->subtype = DEV_FBDEV;
    dev_register(NULL, fb);
}
