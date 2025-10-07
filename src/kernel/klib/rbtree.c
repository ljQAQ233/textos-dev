/*
 * red-black tree implementation
 *  - this implementation doesn't provide an automatical `insert` method but `link` and `fixup`
 */
#include <textos/klib/rbtree.h>

enum
{
    RED = 0,
    BLACK = 1,
};

static rbnode_t *rotate_left(rbtree_t *t, rbnode_t *x)
{
    rbnode_t *y = x->right;
    x->right = y->left;
    if (y->left)
        y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent)
        t->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
    return y;
}

static rbnode_t *rotate_right(rbtree_t *t, rbnode_t *y)
{
    rbnode_t *x = y->left;
    y->left = x->right;
    if (x->right)
        x->right->parent = y;
    x->parent = y->parent;
    if (!y->parent)
        t->root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else
        y->parent->right = x;
    x->right = y;
    y->parent = x;
    return x;
}

static bool isred(rbnode_t *x)
{
    return x && x->color == RED;
}

static void fixup(rbtree_t *t, rbnode_t *z)
{
    while (z != t->root && isred(z->parent))
    {
        rbnode_t *p = z->parent;
        rbnode_t *g = p->parent;
        if (p == g->left)
        {
            rbnode_t *u = g->right;
            if (isred(u))
            {
                p->color = BLACK;
                u->color = BLACK;
                g->color = RED;
                z = g;
            }
            else
            {
                if (z == p->right)
                {
                    z = p;
                    rotate_left(t, z);
                    p = z->parent;
                    g = p->parent;
                }
                p->color = BLACK;
                g->color = RED;
                rotate_right(t, g);
            }
        }
        else
        {
            rbnode_t *u = g->left;
            if (isred(u))
            {
                p->color = BLACK;
                u->color = BLACK;
                g->color = RED;
                z = g;
            }
            else
            {
                if (z == p->left)
                {
                    z = p;
                    rotate_right(t, z);
                    p = z->parent;
                    g = p->parent;
                }
                p->color = BLACK;
                g->color = RED;
                rotate_left(t, g);
            }
        }
    }
    t->root->color = BLACK;
}

static rbnode_t *minimum(rbnode_t *x)
{
    while (x->left)
        x = x->left;
    return x;
}

static void fixup_del(rbtree_t *t, rbnode_t *x)
{
    while (x != t->root && (!x || x->color == BLACK))
    {
        rbnode_t *p = x ? x->parent : NULL;
        if (x == p->left)
        {
            rbnode_t *w = p->right;
            if (isred(w))
            {
                w->color = BLACK;
                p->color = RED;
                rotate_left(t, p);
                w = p->right;
            }
            if ((!w->left || w->left->color == BLACK) &&
                (!w->right || w->right->color == BLACK))
            {
                w->color = RED;
                x = p;
            }
            else
            {
                if (!w->right || w->right->color == BLACK)
                {
                    if (w->left) w->left->color = BLACK;
                    w->color = RED;
                    rotate_right(t, w);
                    w = p->right;
                }
                w->color = p->color;
                p->color = BLACK;
                if (w->right) w->right->color = BLACK;
                rotate_left(t, p);
                x = t->root;
            }
        }
        else
        {
            rbnode_t *w = p->left;
            if (isred(w))
            {
                w->color = BLACK;
                p->color = RED;
                rotate_right(t, p);
                w = p->left;
            }
            if ((!w->right || w->right->color == BLACK) &&
                (!w->left || w->left->color == BLACK))
            {
                w->color = RED;
                x = p;
            }
            else
            {
                if (!w->left || w->left->color == BLACK)
                {
                    if (w->right) w->right->color = BLACK;
                    w->color = RED;
                    rotate_left(t, w);
                    w = p->left;
                }
                w->color = p->color;
                p->color = BLACK;
                if (w->left) w->left->color = BLACK;
                rotate_right(t, p);
                x = t->root;
            }
        }
    }

    if (x)
        x->color = BLACK;
}

static void replace(rbnode_t *n, rbnode_t **pp)
{
    rbnode_t *old = *pp;
    n->color = old->color;
    n->left = old->left;
    n->right = old->right;
    n->parent = old->parent;
    if (old->left)
        old->left->parent = n;
    if (old->right)
        old->right->parent = n;
    *pp = n;
}

void rbtree_link(rbnode_t *n, rbnode_t **pp, rbnode_t *p)
{
    if (*pp)
    {
        replace(n, pp);
        return ;
    }

    n->color = RED;
    n->left = NULL;
    n->right = NULL;
    n->parent = p;
    *pp = n;
}

void rbtree_fixup(rbtree_t *t, rbnode_t *n)
{
    fixup(t, n);
}

void rbtree_delete(rbtree_t *t, rbnode_t *z)
{
    rbnode_t *y = z;
    rbnode_t *x;
    int y_original_color = y->color;

    if (!z->left)
    {
        x = z->right;
        if (x) x->parent = z->parent;
        if (!z->parent)
            t->root = x;
        else if (z == z->parent->left)
            z->parent->left = x;
        else
            z->parent->right = x;
    }
    else if (!z->right)
    {
        x = z->left;
        if (x) x->parent = z->parent;
        if (!z->parent)
            t->root = x;
        else if (z == z->parent->left)
            z->parent->left = x;
        else
            z->parent->right = x;
    }
    else
    {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z)
        {
            if (x) x->parent = y;
        }
        else
        {
            if (x) x->parent = y->parent;
            y->parent->left = x;
            y->right = z->right;
            y->right->parent = y;
        }

        if (!z->parent)
            t->root = y;
        else if (z == z->parent->left)
            z->parent->left = y;
        else
            z->parent->right = y;

        y->parent = z->parent;
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == BLACK && x)
        fixup_del(t, x);
}

static void rbtree_walk(rbnode_t *n, rbcallback_t cb, int mode, int d)
{
    if (!n) return;
    if (mode == RB_PREORDER) cb(d, n);
    rbtree_walk(n->left, cb, mode, d + 1);
    if (mode == RB_INORDER) cb(d, n);
    rbtree_walk(n->right, cb, mode, d + 1);
    if (mode == RB_POSTORDER) cb(d, n);
}

void rbtree_foreach(rbtree_t *t, rbcallback_t cb, int mode)
{
    rbtree_walk(t->root, cb, mode, 0);
}
