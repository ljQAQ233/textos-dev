// sync SHARED pages to files in disks
#include <cpu.h>
#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/klib/list.h>
#include <textos/mm/map.h>
#include <textos/mm/mman.h>
#include <textos/task.h>

list_t vmm_sync_list = LIST_INIT(vmm_sync_list);

int vmm_sync_regst(struct vm_area *vma)
{
    assert(vma->flgs & MAP_SHARED);
    assert(vma->label == MAPL_FILE);
    if (vma->flgs & MAP_SYNCING) return 0;

    struct vm_sync *vms = malloc(sizeof(struct vm_sync));
    if (!vms) return -ENOMEM;

    vms->vma = vma;
    vms->pgt = task_current()->pgt;
    list_insert(&vmm_sync_list, &vms->list);
    vma->flgs |= MAP_SYNCING;
    return 0;
}

static void scan_cb(addr_t va, void *data)
{
    struct vm_area *vma = data;
    size_t off = va - vma->s;
    size_t rem = vma->obj.node->siz - vma->obj.foff;
    vfs_write(vma->obj.node, (void *)va, MIN(PAGE_SIZE, rem),
              vma->obj.foff + off, &(struct fs_openctx){0});
}

void vmm_sync_area(struct vm_sync *vms)
{
    addr_t cr3 = read_cr3();
    write_cr3(vms->pgt);
    vmap_scan_dirty(vms->vma->s, vms->vma->t, scan_cb, vms->vma);
    write_cr3(cr3);
}

void __vmm_syncproc()
{
    list_t *ptr;
    struct vm_sync *vms;
    LIST_FOREACH(ptr, &vmm_sync_list)
    {
        vms = CR(ptr, struct vm_sync, list);
        vmm_sync_area(vms);
    }
}
