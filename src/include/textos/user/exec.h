#pragma once

typedef struct
{
    void *entry;
    char *path;
} exeinfo_t;

int elf_load (char *path, exeinfo_t *exe);

