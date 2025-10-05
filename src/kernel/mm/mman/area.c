#include <textos/mm/mman.h>

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
