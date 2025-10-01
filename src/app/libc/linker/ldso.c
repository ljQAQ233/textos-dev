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
    dprintf(2, "try %s <program> [...] instead\n", prog);
    return 1;
}

static struct dl _self;
static struct dl *self;

static int fixinfo(char *path)
{
    struct stat sb;
    if (stat(path, &sb) < 0) {
        dlerr("stat error");
        return -1;
    }
    self = &_self;
    self->flags = -1;
    self->dev = sb.st_dev;
    self->ino = sb.st_ino;
    self->path = path;
    self->refcnt = 1;
    self->fd = -1;
    return 0;
}

static int fixaddr()
{
    void *pmap = (void *)getauxval(AT_PHDR);
    uintptr_t dmap = -1;
    int phent = getauxval(AT_PHENT);
    int phnum = getauxval(AT_PHNUM);
    for (int i = 0 ; i < phnum ; i++)
    {
        Elf64_Phdr *ph = pmap + i * phent;
        if (ph->p_type == PT_PHDR)
            self->virt = pmap - ph->p_vaddr;
        if (ph->p_type == PT_DYNAMIC)
            dmap = ph->p_vaddr;
    }
    if (dmap == -1)
    {
        dlerr("can not find .dynmic");
        return -1;
    }
    self->dynamic = self->virt + dmap;
    self->entry = (void *)getauxval(AT_ENTRY);
    return 0;
}

int main(int argc, char *argv[], char *envp[])
{
    /*
     * is the dynamic loader executed by commands?
     */
    {
        Elf64_Ehdr *eh = (void *)&__ehdr_start;
        void *ph = &__ehdr_start + eh->e_phoff;
        if ((void *)getauxval(AT_PHDR) == ph)
        {
            if (argc <= 1)
                return help(argv[0]);
            *(long *)argv = --argc;
            argv++;

            self = loadlib(argv[0], RTLD_LAZY);
            goto run;
        }
    }

    if (fixinfo(argv[0]) < 0 ||
        fixaddr() < 0 ||
        dolkp(self) < 0)
    {
        dprintf(2, "ld.so: \n", dlerror());
        return 1;
    }

run:
    r_addlib(self);
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
