#include <textos/mm/heap.h>
#include <textos/mm/mman.h>

/*
 * vma design:
 *   - premises:
 *     - the pmm does not provide fine-grained page frame
 *       management like linux (no reference counting)
 *     - physical page references are only incremented during fork
 * cow design:
 *   - the vma tracks the state of cow pages; pages are set to read-only,
 *     and a copy is made upon a #PF (page fault) event
 */

vm_area_t *vmm_new_vma(vm_area_t *old)
{
    vm_area_t *vma = malloc(sizeof(vm_area_t));
    if (old)
    {
        *vma = *old;
        vma->ppgs = vmm_ppg_new(old->ppgs);
    }
    else
        vma->ppgs = vmm_ppg_new(0);
    return vma;
}

void vmm_free_vma(vm_area_t *vma)
{
    vmm_ppg_clear(vma);
    free(vma);
}

/*
 * a space consists of many vm areas
 */
vm_space_t *vmm_new_space(vm_space_t *old)
{
    vm_space_t *sp = malloc(sizeof(vm_space_t));
    list_init(&sp->list);
    sp->tree = RBTREE_INIT();
    if (old)
    {
        list_t *ptr;
        LIST_FOREACH(ptr, &old->list)
        {
            vm_area_t *o = CR(ptr, vm_area_t, list);
            vm_area_t *a = vmm_new_vma(o);
            vmm_sp_regst(sp, a);
        }
    }
    return sp;
}

void vmm_free_space(vm_space_t *sp)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &sp->list)
    {
        vm_area_t *o = CR(ptr, vm_area_t, list);
        vmm_free_vma(o);
    }
}

void vmm_sp_regst(vm_space_t *sp, vm_area_t *vma)
{
    rbtree_t *t = &sp->tree;
    rbnode_t *p = NULL;
    rbnode_t **pp = &t->root;
    while (*pp) {
        p = *pp;
        vm_area_t *a = CR(*pp, vm_area_t, node);
        if (a->s < vma->s)
            pp = &(*pp)->right;
        else if (a->s > vma->s)
            pp = &(*pp)->left;
    }

    rbtree_link(&vma->node, pp, p);
    rbtree_fixup(t, &vma->node);

    vm_area_t *prev = vmm_sp_upperbound(sp, vma->s);
    if (prev != NULL)
        list_insert_after(&prev->list, &vma->list);
    else
        list_insert_before(&sp->list, &vma->list);
}

static void rbcb(int d, rbnode_t *ptr)
{
    vm_area_t *vma = CR(ptr, vm_area_t, node);
    for (int i = 0 ; i < d ; i ++)
        dprintk(K_LOGK, " ");
    dprintk(K_LOGK, "- [%d] %p\n", ptr->color, vma->s);
}

#include <textos/assert.h>

void vmm_sp_display(vm_space_t *sp)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &sp->list)
    {
        vm_area_t *vma = CR(ptr, vm_area_t, list);
        DEBUGK(K_LOGK, "%p-%p prot=%x\n", vma->s, vma->t, vma->prot);
        ASSERTK(!!vma->t);
    }
    rbtree_foreach(&sp->tree, rbcb, RB_PREORDER);
}

void mmap_unreg(vm_space_t *sp, vm_area_t *vma)
{
    rbtree_delete(&sp->tree, &vma->node);
    list_remove(&vma->list);
}

vm_area_t *vmm_sp_lowerbound(vm_space_t *sp, addr_t addr)
{
    rbnode_t *p = sp->tree.root;
    vm_area_t *c = NULL;
    while (p) {
        /*
         * 要找出 第一个不小于 addr 的, 比较 起始地址. 满足条件的看看还有没有更小的,
         * 往左走. 否则需要一个更大的, 右边请 ~
         */
        vm_area_t *a = CR(p, vm_area_t, node);
        if (a->s >= addr) {
            c = a;
            p = p->left;
        } else {
            p = p->right;
        }
    }
    return c;
}

vm_area_t *vmm_sp_upperbound(vm_space_t *sp, addr_t addr)
{
    rbnode_t *p = sp->tree.root;
    vm_area_t *c = NULL;
    while (p) {
        vm_area_t *a = CR(p, vm_area_t, node);
        if (a->s > addr) {
            c = a;
            p = p->left;
        } else {
            p = p->right;
        }
    }
    return c;
}

vm_area_t *vmm_sp_containing(vm_space_t *sp, addr_t addr)
{
    rbnode_t *p = sp->tree.root;
    vm_area_t *c = NULL;
    while (p) {
        vm_area_t *a = CR(p, vm_area_t, node);
        if (addr >= a->s && addr < a->t) {
            c = a;
            break;
        } else if (addr < a->s) {
            p = p->left;
        } else {
            p = p->right;
        }
    }
    return c;
}
