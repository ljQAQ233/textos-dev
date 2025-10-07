#include <textos/mm/heap.h>
#include <textos/mm/mman.h>

vm_area_t *mmap_new_vma(vm_area_t *old)
{
    vm_area_t *vma = malloc(sizeof(vm_area_t));
    if (old)
        *vma = *old;
    return vma;
}

void mmap_regst(vm_space_t *sp, vm_area_t *vma)
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

    vm_area_t *prev = mmap_upperbound(sp, vma->s);
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

void mmap_display(vm_space_t *sp)
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

vm_area_t *mmap_lowerbound(vm_space_t *sp, addr_t addr)
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

vm_area_t *mmap_upperbound(vm_space_t *sp, addr_t addr)
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

vm_area_t *mmap_containing(vm_space_t *sp, addr_t addr)
{
    rbnode_t *p = sp->tree.root;
    vm_area_t *c = NULL;
    mmap_display(sp);
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
