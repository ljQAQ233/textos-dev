#include <textos/fs.h>
#include <textos/mm.h>
#include <textos/klib/stack.h>

/*
 * 挂载点设计:
 *   - base - 如何挂载?
 *     - dir->child = root, root->parent = dir->parent
 *       - 好处: 查找 .. 的时候可以忽略掉挂载点
 *       - 坏处: 语义混乱 root->name 不方便访问
 *       - 妥协: dir->child = parent, root->parent = dir,
 *               物理文件系统驱动不能直接访问其他 node
 *               readdir 如果要访问 ./.. 交给 vfs_getprt / dir_emit_dot.
 *               这里依靠 node 可以直接获取其父节点的前提是 vfs 树上的节点不会随意回收
 *   - overlay - 被覆盖的挂载点下的目录如果被引用?
 *     - stack_t 驱动
 * 细节:
 *   - stat 挂载点 -> 得到的是 伪根目录 而不是 挂载目录
 */

// mount overlay
typedef struct
{
    stack_t chd;
} mount_t;

int vfs_mount(node_t *dir, node_t *root)
{
    if (!dir->mount)
        dir->mount = malloc(sizeof(mount_t));

    mount_t *mnt = dir->mount;
    stack_push(&mnt->chd, dir->child);
    dir->child = root;
    dir->attr |= FSA_MNT;
    root->parent = dir;
    return 0;
}

int vfs_mount_to(char *path, node_t *root, int mode)
{
    int ret = 0;
    node_t *dir;

    ret = vfs_open(NULL, path, O_DIRECTORY | O_CREAT, mode, &dir);
    if (ret < 0)
        return ret;

    return vfs_mount(dir, root);
}

int vfs_umount(node_t *dir)
{
    mount_t *mnt = dir->mount;
    node_t *root = dir->child;
    root->parent = root;
    dir->child = stack_top(&mnt->chd),
                 stack_pop(&mnt->chd);
    if (stack_siz(&mnt->chd) == 0)
        dir->attr &= ~FSA_MNT;

    return 0;
}

bool vfs_ismount(node_t *n)
{
    return n->attr & FSA_MNT;
}

bool vfs_isaroot(node_t *n)
{
    return vfs_ismount(n->parent);
}
