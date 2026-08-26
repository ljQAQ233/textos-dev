#include <textos/errno.h>
#include <textos/task.h>

static bool insgrp(task_t *tsk, gid_t gid)
{
    for (gid_t *sg = tsk->supgids; *sg != -1; sg++)
        if (gid == *sg) return true;
    return false;
}

int vfs_chown(node_t *file, uid_t owner, gid_t group)
{
    task_t *tsk = task_current();

    /*
     * Only processes with euid equals to the file->uid or procsses with
     * appropriate privilege can change the ownership of that file. If
     * _POSIX_CHOWN_RESTRICTED is in effect:
     *   - only privileged proc can change ownership
     *   - file->uid == tsk->euid, and group is equal to the callers' egid or
     * one of its supplementary gids. we keep _POSIX_CHOWN_RESTRICTED enabled.
     */
    bool ochg = owner != -1 && owner != file->uid;
    bool gchg = group != -1 && group != file->gid;
    if (tsk->euid != 0) {
        if (ochg) {
            // _POSIX_CHOWN_RESTRICTED
            return -EPERM;
        }

        if (gchg && tsk->egid != group && !insgrp(tsk, group)) return -EPERM;
    }

    if (ochg || gchg) {
        // handle -1
        if (!ochg) owner = file->uid;
        if (!gchg) group = file->gid;
        int ret = file->sb->op->chown(file, owner, group, tsk->euid == 0);
        if (ret < 0) return ret;
    }
    return 0;
}
