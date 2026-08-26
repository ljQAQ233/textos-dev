node_t *__vfs_root = NULL;

bool __vfs_rootset(node_t *root)
{
    if (!__vfs_root) __vfs_root = root;
    return __vfs_root == root;
}

