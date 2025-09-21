/*
 * dynamic linker
 *   - static-linked with `libc.a` and make it minimum
 */
#include <stdio.h>
#include <dlfcn.h>
#include <sys/auxv.h>
#define __NEED_linker
#include "linker.c"
#include "resolve.c"

extern char __ehdr_start;

static int help(const char *prog)
{
    dprintf(1, "try %s <program> [...] instead\n", prog);
    return 1;
}

int main(int argc, const char *argv[], const char *envp[])
{
    Elf64_Ehdr *eh = (void *)&__ehdr_start;
    void *ph = &__ehdr_start + eh->e_phoff;
    if ((void *)getauxval(AT_PHDR) == ph)
    {
        if (argc <= 1)
            return help(argv[0]);
        *(long *)argv = --argc;
        argv++;
    }

    struct dl *self = loadlib(argv[0], RTLD_LAZY);
    __dlself = self;
    if (!self)
    {
        dprintf(1, "%s\n", dlerror());
        return 1;
    }
    void *entry = self->entry;
    asm volatile(
        "mov %0, %%rsp\n"
        "jmp *%1\n"
        :
        : "r"(argv-1), "r"(entry)
        : "memory"
    );

    return 0;
}
