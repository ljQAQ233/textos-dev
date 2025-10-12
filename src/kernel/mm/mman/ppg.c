#include <textos/mm/pmm.h>
#include <textos/mm/heap.h>
#include <textos/mm/mman.h>
#include <textos/assert.h>

static htkey_t hash(addr_t pa)
{
    pa >>= PAGE_SHIFT;
    return (htkey_t)pa;
}

vm_ppg_t *vmm_ppg_new(vm_ppg_t *old)
{
    if (old)
    {
        hlist_node_t *ptr;
        vm_ppgrec_t *record;
        HTABLE_FORANY(ptr, &old->ht)
        {
            record = CR(ptr, vm_ppgrec_t, hlist);
            record->refcnt += 1;
        }
        return old;
    }
    vm_ppg_t *ppgs = malloc(sizeof(vm_ppg_t));
    htable_init(&ppgs->ht, 32);
    return ppgs;
}

void vmm_ppg_regst(vm_area_t *vma, addr_t pa, int flag)
{
    vm_ppgrec_t *record = malloc(sizeof(vm_ppgrec_t));
    record->pa = pa;
    record->flag = flag;
    record->refcnt = 1;
    htkey_t key = hash(pa);
    htable_add(&vma->ppgs->ht, &record->hlist, key);
}

void vmm_ppg_clear(vm_area_t *vma)
{
    hlist_node_t *ptr;
    vm_ppgrec_t *record;
    HTABLE_FORANY(ptr, &vma->ppgs->ht)
    {
        record = CR(ptr, vm_ppgrec_t, hlist);
        record->refcnt -= 1;
        if (!record->refcnt && !(record->flag & PPG_FIXED))
            pmm_freepages(record->pa, 1);
    }
}
