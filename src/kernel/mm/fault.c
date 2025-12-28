#include <cpu.h>
#include <irq.h>
#include <intr.h>
#include <textos/mm.h>
#include <textos/task.h>
#include <textos/mm/vmm.h>
#include <textos/mm/mman.h>
#include <textos/klib/string.h>

enum err
{
    PF_P = (1 << 0),
    PF_WRITE = (1 << 1),
    PF_USER = (1 << 2),
    PF_RSVD = (1 << 3),
    PF_ID = (1 << 4),
    PF_PK = (1 << 5),
    PF_SS = (1 << 6),
    PF_HALT = (1 << 7),
    PF_SGX = (1 << 15),
};

#define align_dn(x, y) ((y) * (x / y))

__INTR_HANDLER(pagefault_handler)
{
    task_t *tsk = task_current();
    addr_t addr = read_cr2();
    vm_space_t *sp = tsk->vsp;
    vm_area_t *vma = vmm_sp_containing(sp, addr);
    DEBUGK(K_TRACE, "page fault at %p (err=%x)\n", addr, errcode);
    int i = 0;
    if (!vma)
        goto segv;
    if (~errcode & PF_P)
    {
        if (vma->label == MAPL_FILE)
        {
            size_t off = addr - vma->s;
            size_t pgbase = align_dn(addr, PAGE_SIZE);
            size_t foff = vma->obj.foff + pgbase - vma->s;
            size_t siz = MIN(vma->obj.node->siz - foff, PAGE_SIZE);
            addr_t ppg = pmm_allocpages(1);
            vmap_map((addr_t)ppg, pgbase, 1, PE_P | PE_US | PE_RW);
            vfs_read(vma->obj.node, (void *)pgbase, siz, foff);
            vmap_map((addr_t)ppg, pgbase, 1, PE_P | PE_US | mapprot(vma->prot));
            vmm_ppg_regst(vma, ppg, 0);
            return;
        }
        if (vma->flgs & MAP_ANON)
        {
            size_t pgbase = align_dn(addr, PAGE_SIZE);
            addr_t ppg = pmm_allocpages(1);
            vmap_map((addr_t)ppg, pgbase, 1, PE_P | PE_US | mapprot(vma->prot));
            vmm_ppg_regst(vma, ppg, 0);
            return;
        }
    }
segv:
    if (i != 1)
    {
        kill(tsk->pid, SIGSEGV);
    }
}

void pagefault_init()
{
    intr_register(INT_PF, pagefault_handler);
}
