#include <textos/errno.h>
#include <textos/fs.h>
#include <textos/task.h>

int vfs_permission(node_t *n, int want)
{
    task_t *tsk = task_current();
    mode_t bits = n->mode;
    if (tsk->euid == n->uid)
        bits >>= 6;
    else if (tsk->egid == n->gid)
        bits >>= 3;
    else
        bits >>= 0;

    if ((want & MAY_READ) && !(bits & 4)) return -EACCES;
    if ((want & MAY_WRITE) && !(bits & 2)) return -EACCES;
    if ((want & MAY_EXEC) && !(bits & 1)) return -EACCES;
    return 0;
}

