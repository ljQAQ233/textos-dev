#include <cpu.h>
#include <intr.h>
#include <irq.h>
#include <textos/assert.h>
#include <textos/klib/string.h>
#include <textos/mm/mman.h>
#include <textos/mm/pmm.h>
#include <textos/mm/pvpage.h>
#include <textos/mm/vmm.h>
#include <textos/task.h>

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
    if (!vma) goto segv;
    if (~errcode & PF_P) {
        if (vma->label == MAPL_FILE) {
            size_t pgbase = align_dn(addr, PAGE_SIZE);
            size_t foff = vma->obj.foff + pgbase - vma->s;
            size_t siz = MIN(vma->obj.node->siz - foff, PAGE_SIZE);
            struct fs_openctx ctx = {0};
            addr_t ppg = pmm_allocpages(1);
            vmap_map((addr_t)ppg, pgbase, 1, PE_P | PE_US | PE_RW);
            vfs_read(vma->obj.node, (void *)pgbase, siz, foff, &ctx);
            vmap_map((addr_t)ppg, pgbase, 1, PE_P | PE_US | mapprot(vma->prot));
            vmm_ppg_regst(vma, ppg, 0);
            return;
        }
        if (vma->flgs & MAP_ANON) {
            size_t pgbase = align_dn(addr, PAGE_SIZE);
            addr_t ppg = pmm_allocpages(1);
            vmap_map((addr_t)ppg, pgbase, 1, PE_P | PE_US | mapprot(vma->prot));
            vmm_ppg_regst(vma, ppg, 0);
            return;
        }
    }
    if (errcode & PF_WRITE) {
        // 在实现 COW 的时候, 由于我没有为每一个页维护一个页框, 所以每一个 vma
        // 区间都会维护一个 ppgs 的 hashtable, 用来记录物理页的引用, 这要求 vma
        // 可以唯一对应一个 pagetable. 于是, 在 copy_pgt 的时候, 就可以,
        // 只将一部分的 双方可写入 的内容给设置成 只读不写, 而相应的 ppgs 的
        // 引用计数仅仅需要整体 +1, 而不需要 copy_pgt 进行 一个一个的注册.
        // 这是一个 prerequisite!!!
        if (vma->prot & PROT_WRITE) {
            size_t pgbase = align_dn(addr, PAGE_SIZE);
            addr_t ppg = vmap_query(pgbase);
            ASSERTK(ppg != 0);
            vm_ppgrec_t *pgrec = vmm_ppg_query(vma, ppg);
            assert(pgrec != NULL);
            if (pgrec->refcnt == 1) {
                vmap_map(ppg, pgbase, 1, PE_P | PE_US | mapprot(vma->prot));
                vmm_ppg_reparent(pgrec, vma);
                return;
            }

            pgrec->refcnt -= 1;
            addr_t npg;
            {
                void *vpg, *vnpg;
                npg = pmm_allocpages(1);
                while (make_pvpage((u64)ppg, &vpg) < 0)
                    ;
                while (make_pvpage((u64)npg, &vnpg) < 0)
                    ;
                memcpy(vnpg, vpg, PAGE_SIZ);
                break_pvpage(vpg);
                break_pvpage(vnpg);
            }
            vmm_ppg_regst(vma, npg, 0);
            vmap_map(npg, pgbase, 1, PE_P | PE_US | mapprot(vma->prot));
            DEBUGK(K_TRACE, "page fault cow at %p\n", addr);
            return;
        }
    }
segv:
    if (i != 1) {
        fkill(tsk->pid, SIGSEGV);
    }
}

void pagefault_init()
{
    intr_register(INT_PF, pagefault_handler);
}
