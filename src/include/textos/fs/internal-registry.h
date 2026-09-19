#pragma once

struct fs_registry
{
    char *name;
    int id;
    superblk_t *(*init)(devst_t *hd);
};
