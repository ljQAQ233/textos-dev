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

vm_ppgrec_t *vmm_ppg_query(vm_area_t *vma, addr_t pa)
{
    hlist_node_t *ptr;
    vm_ppgrec_t *record;
    htkey_t key = hash(pa);
    HTABLE_FOREACH(ptr, &vma->ppgs->ht, key)
    {
        record = CR(ptr, vm_ppgrec_t, hlist);
        if (record->pa == pa)
            return record;
    }
    return NULL;
}

void vmm_ppg_refer(vm_ppgrec_t *rec)
{
    rec->refcnt++;
}

void vmm_ppg_unref(vm_ppgrec_t *rec)
{
    rec->refcnt -= 1;
    if (!rec->refcnt && !(rec->flag & PPG_FIXED))
        pmm_freepages(rec->pa, 1);
}

void vmm_ppg_clear(vm_area_t *vma)
{
    hlist_node_t *ptr;
    vm_ppgrec_t *record;
    HTABLE_FORANY(ptr, &vma->ppgs->ht)
    {
        record = CR(ptr, vm_ppgrec_t, hlist);
        vmm_ppg_unref(record);
    }
}
