#pragma once

#include "sys/cdefs.h"
#include <textos/dev.h>

extern node_t *__vfs_root;

// vroot.c
bool __vfs_rootset(node_t *root);

// utils

static inline char *fs_path_next(const char *p)
{
    while (*p != '/' && *p)
        p++;
    while (*p == '/')
        p++;
    return (char *)p;
}

static inline bool fs_path_isdir(const char *p)
{
    bool res = false;
    for (; p && *p; p++) {
        if (*p == '/') res = true;
        if (*p == ' ') continue;
        res = false;
    }
    return res;
}

extern size_t strlen(const char *str);
extern char *strchr(const char *str, int c);

static inline bool fs_name_cmp(const char *A, const char *B)
{
    size_t lenA = MIN(strlen(A), strchr(A, '/') - A);
    size_t lenB = MIN(strlen(B), strchr(B, '/') - B);
    if (lenA != lenB) return false;
    for (size_t i = 0; i < lenB; i++)
        if (A[i] != B[i]) return false;
    return true;
}

extern void noopt_handler();

#include <textos/klib/string.h>
#include <textos/mm/heap.h>
#include <textos/task.h>
/*
 * vfs in-memory node operations used by physical file systems
 */
static int __unused vfs_m_mknod(node_t *prt, char *name, dev_t rdev, int mode, node_t **result, ino_t ino)
{
    node_t *chd = calloc(sizeof(*chd));
    chd->name = strdup(name);
    chd->attr = 0;
    chd->siz = 0;
    chd->uid = task_current()->euid;
    chd->gid = task_current()->egid;
    chd->ino = ino;
    chd->mode = mode & (S_IFMT | (mode &~ task_current()->umask));
    chd->atime = chd->mtime = chd->ctime = arch_time_now();
    chd->dev = prt->dev;
    chd->rdev = rdev;
    chd->pdata = NULL;
    chd->sb = prt->sb;
    chd->opts = prt->opts;
    vfs_regst(chd, prt);

    *result = chd;
    return 0;
}

static int __unused vfs_m_chown(node_t *this, uid_t owner, gid_t group, bool ap)
{
    if (!ap && S_ISREG(this->mode))
        if (this->mode & (S_IXUSR | S_IXGRP | S_IXOTH))
            this->mode &= ~(S_ISUID | S_ISGID);
    this->uid = owner;
    this->gid = group;
    this->ctime = arch_time_now();
    return 0;
}

static int __unused vfs_m_chmod(node_t *this, mode_t mode, bool clrsgid)
{
    this->mode &= ~07777;
    this->mode |= mode & 07777;
    if (clrsgid)
        this->mode &= ~S_IXGRP;
    this->ctime = arch_time_now();
    return 0;
}
