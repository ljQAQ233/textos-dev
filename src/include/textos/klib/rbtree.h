#pragma once

typedef u64 rbkey_t;

typedef struct rbnode
{
    int color;
    struct rbnode *left;
    struct rbnode *right;
    struct rbnode *parent;
} rbnode_t;

typedef void (*rbcallback_t)(int d, rbnode_t *node);

typedef struct rbtree
{
    rbnode_t *root;
} rbtree_t;

#define RBTREE_INIT() ((rbtree_t){ 0 })

/**
 * @brief link a child node to the parent, if the location has been occupied, the former one will be replaced
 * 
 * @param n  node to be inserted
 * @param pp location to be inserted on, a pointer to left or right pointer of parent node
 * @param p  pointer to parent node
 */
void rbtree_link(rbnode_t *n, rbnode_t **pp, rbnode_t *p);

/**
 * @brief called after linking to fixup the tree
 * 
 * @param t tree
 * @param n node inserted before
 */
void rbtree_fixup(rbtree_t *t, rbnode_t *n);

/**
 * @brief delete a node from t
 * 
 * @param t tree
 * @param z node to be deleted
 */
void rbtree_delete(rbtree_t *t, rbnode_t *z);

enum
{
    RB_PREORDER,
    RB_INORDER,
    RB_POSTORDER
};

/**
 * @brief foreach node
 */
void rbtree_foreach(rbtree_t *t, rbcallback_t cb, int mode);
