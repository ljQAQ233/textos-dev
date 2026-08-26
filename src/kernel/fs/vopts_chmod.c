#include <textos/errno.h>
#include <textos/task.h>

static bool insgrp(task_t *tsk, gid_t gid)
{
    for (gid_t *sg = tsk->supgids; *sg != -1; sg++)
        if (gid == *sg) return true;
    return false;
}

int vfs_chmod(node_t *file, mode_t mode)
{
    task_t *tsk = task_current();
    mode &= 07777;

    if (tsk->euid != 0 && tsk->euid != file->uid) return -EPERM;
    /*
     * For security: S_ISGID bit is preserved only if privileged or the gid of
     * this file is in the process's supplementary group list.
     */
    bool clr = true;
    if (tsk->euid == 0)
        clr = false;
    else if (insgrp(tsk, file->gid))
        clr = false;
    int ret = file->sb->op->chmod(file, mode, clr);
    if (ret < 0) return ret;
    return 0;
}
