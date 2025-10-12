#pragma once

typedef struct
{
    void *entry;
    char *path;
    uintptr_t a_phdr;
    uintptr_t a_phent;
    uintptr_t a_phnum;
    uintptr_t a_base;
    uintptr_t a_notelf;
} exeinfo_t;

int elf_load(char *path, exeinfo_t *exe);
