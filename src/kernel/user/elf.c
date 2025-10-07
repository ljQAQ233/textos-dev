#include <textos/fs.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>
#include <textos/mm/mman.h>
#include <textos/errno.h>
#include <textos/assert.h>
#include <textos/user/elf.h>
#include <textos/user/exec.h>
#include <textos/klib/string.h>

#define ckerr(x) do { if ((ret = (x)) < 0) goto err; } while(0)

static inline int dochk(void *map)
{
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)map;
    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || 
        hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 ||
        hdr->e_ident[EI_MAG3] != ELFMAG3)
        return -ENOEXEC;
    if (hdr->e_machine != EM_X86_64)
        return -ENOEXEC;
    // TODO: static-pie program
    if (hdr->e_type != ET_EXEC)
        return -ENOEXEC;
    return 0;
}

#define align_up(x, y) ((y) * ((x + y - 1) / y))
#define align_dn(x, y) ((y) * (x / y))

static int domap(node_t *n, exeinfo_t *exe)
{
    int ret;
    Elf64_Ehdr _ehdr;
    Elf64_Ehdr *eh = &_ehdr;
    ckerr(vfs_read(n, &_ehdr, sizeof(_ehdr), 0));
    ckerr(dochk(eh));
    void *pmap = malloc(eh->e_phnum * eh->e_phentsize);
    ckerr(vfs_read(n, pmap, eh->e_phnum * eh->e_phentsize, eh->e_phoff));

    /*
     * determine the range of virtual address.
     */
    int lp_segs = 0;
    int lp_okay = 0;
    for (int i = 0 ; i < eh->e_phnum ; i++)
    {
        Elf64_Phdr *ph = pmap + i * eh->e_phentsize;
        if (ph->p_type == PT_LOAD)
            lp_segs++;
    }
    DEBUGK(K_LOGK, "elf file %s\n", n->name);
    DEBUGK(K_LOGK, "total %d segs\n", lp_segs);
    if (lp_segs == 0)
    {
        ret = -ENOEXEC;
        goto err;
    }

    void *base = 0;
    for ( ; lp_okay < eh->e_phnum ; lp_okay++)
    {
        Elf64_Phdr *ph = pmap + lp_okay * eh->e_phentsize;
        if (ph->p_type != PT_LOAD)
            continue;
        uintptr_t foff = align_dn(ph->p_offset, ph->p_align);
        uintptr_t fend = align_up(ph->p_offset + ph->p_filesz, ph->p_align);
        uintptr_t fsize = fend - foff;
        uintptr_t moff = align_dn(ph->p_vaddr, ph->p_align);
        uintptr_t mend = align_up(ph->p_vaddr + ph->p_memsz, ph->p_align);
        uintptr_t msize = mend - moff;
        int prot = ((ph->p_flags & PF_R) ? PROT_READ : 0)
            | ((ph->p_flags & PF_W) ? PROT_WRITE : 0)
            | ((ph->p_flags & PF_X) ? PROT_EXEC : 0);
        vm_region_t vm = {
            .va = (addr_t)base + moff,
            .num = (size_t)(DIV_ROUND_UP(fsize, PAGE_SIZ)),
            .prot = prot,
            .flgs = MAP_FIXED | MAP_PRIVATE,
            .foff = foff,
            .fnode = n,
            .ppgs = NULL,
        };
        ASSERTK(mmap_file(&vm) >= 0);

        if (ph->p_memsz > ph->p_filesz && (prot & PROT_WRITE))
        {
            uintptr_t diff = msize - fsize;
            if (diff)
            {
                void *rempages = base + moff + fsize;
                vm_region_t vm = {
                    .va = (addr_t)rempages,
                    .num = (size_t)(DIV_ROUND_UP(diff, PAGE_SIZ)),
                    .prot = PROT_READ | PROT_WRITE | prot,
                    .flgs = MAP_FIXED | MAP_PRIVATE | MAP_ANON,
                    .foff = 0,
                    .fnode = NULL,
                    .ppgs = NULL,
                };
                ASSERTK(mmap_anon(&vm) >= 0);
            }
            char *zero = base + ph->p_vaddr + ph->p_filesz;
            memset(zero, 0, ph->p_memsz - ph->p_filesz);
        }
        DEBUGK(K_LOGK, "  %p -> %p, size = %lx, prot = %x\n", foff, base + moff, msize, prot);
    }

    exe->entry = (void *)eh->e_entry;
err:
    if (pmap)
        free(pmap);
    return MIN(ret, 0);
}


int elf_load(char *path, exeinfo_t *exe)
{
    int ret;
    node_t *n;
    exe->path = strdup(path);
    ckerr(vfs_open(NULL, path, 0, 0, &n));
    ckerr(domap(n, exe));
err:
    return MIN(ret, 0);
}
