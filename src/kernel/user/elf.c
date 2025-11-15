#include <textos/fs.h>
#include <textos/mm.h>
#include <textos/task.h>
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
    // then check its type
    if (hdr->e_type == ET_EXEC)
        return 0;
    if (hdr->e_type == ET_DYN)
        return 1;
    return -ENOEXEC;
}

#define align_up(x, y) ((y) * ((x + y - 1) / y))
#define align_dn(x, y) ((y) * (x / y))

static int domap(node_t *n, exeinfo_t *exe, bool allow_ld)
{
    int ret;
    int isdyn;
    void *base = 0;
    Elf64_Ehdr _ehdr;
    Elf64_Ehdr *eh = &_ehdr;
    uintptr_t pmapself = 0;
    ckerr(vfs_read(n, &_ehdr, sizeof(_ehdr), 0));
    ckerr(isdyn = dochk(eh));
    void *pmap = malloc(eh->e_phnum * eh->e_phentsize);
    ckerr(vfs_read(n, pmap, eh->e_phnum * eh->e_phentsize, eh->e_phoff));

    int lp_segs = 0;
    int lp_okay = 0;
    uintptr_t lp_max = 0;
    uintptr_t lp_min = -1;
    uintptr_t lp_pages;
    for (int i = 0 ; i < eh->e_phnum ; i++)
    {
        Elf64_Phdr *ph = pmap + i * eh->e_phentsize;
        if (ph->p_type == PT_LOAD)
        {
            lp_segs++;
            uintptr_t dptr = align_dn(ph->p_vaddr, ph->p_align);
            uintptr_t uptr = align_up(ph->p_vaddr + ph->p_memsz, ph->p_align);
            if (dptr < lp_min) lp_min = dptr;
            if (uptr > lp_max) lp_max = uptr;
        }
        if (ph->p_type == PT_INTERP)
        {
            /*
             * the path is stored in .interp
             *   - use `readelf -p .interp xxx` to check it
             */
            if (!allow_ld)
            {
                ret = -ENOENT;
                goto err;
            }
            exe->interp = malloc(ph->p_filesz);
            ckerr(vfs_read(n, exe->interp, ph->p_filesz, ph->p_offset));
        }
    }
    if (lp_segs == 0)
    {
        ret = -ENOEXEC;
        goto err;
    }
    lp_pages = DIV_ROUND_UP(lp_max - lp_min, PAGE_SIZE);
    if (isdyn)
        base = (void *)vmm_fitaddr(0, lp_pages);
    DEBUGK(K_LOGK, "elf file %s\n", n->name);
    DEBUGK(K_LOGK, "total %d segs\n", lp_segs);
    DEBUGK(K_LOGK, "interpreter in %s\n", exe->interp ? exe->interp : "none");
    DEBUGK(K_LOGK, "taken up %d pages at %p\n", lp_pages, base);
    for ( ; lp_okay < eh->e_phnum ; lp_okay++)
    {
        Elf64_Phdr *ph = pmap + lp_okay * eh->e_phentsize;
        if (ph->p_type != PT_LOAD)
            continue;
        if (ph->p_offset <= eh->e_phoff &&
            ph->p_offset + ph->p_filesz >= eh->e_phoff)
            pmapself = (uintptr_t)base + eh->e_phoff - ph->p_offset + ph->p_vaddr;
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
                };
                ASSERTK(mmap_anon(&vm, MAPL_BSS) >= 0);
            }
            char *zero = base + ph->p_vaddr + ph->p_filesz;
            memset(zero, 0, ph->p_memsz - ph->p_filesz);
        }
        DEBUGK(K_LOGK, "  %p -> %p, size = %lx, prot = %x\n", foff, base + moff, msize, prot);
    }

    exe->base = base;
    exe->entry = base + eh->e_entry;
    exe->a_notelf = 0;
    if (isdyn && exe->interp)
    {
        exe->a_phdr = pmapself;
        exe->a_phent = eh->e_phentsize;
        exe->a_phnum = eh->e_phnum;
        exeinfo_t ld;
        ckerr(elf_load(exe->interp, &ld, false));
        exe->dlstart = ld.entry;
        exe->a_base = (uintptr_t)ld.base;
    }
err:
    if (pmap)
        free(pmap);
    return MIN(ret, 0);
}

/*
 * load and generate infomation
 *   - entrypoint
 *   - auxiliary vector
 *     - note that PHDR always stored in PT_LOAD if it is expected to be executable
 */
int elf_load(char *path, exeinfo_t *exe, bool allow_ld)
{
    int ret;
    node_t *n;
    memset(exe, 0, sizeof(exeinfo_t));
    exe->path = strdup(path);
    ckerr(vfs_open(task_current()->pwd, path, 0, 0, &n));
    ckerr(domap(n, exe, allow_ld));
err:
    return MIN(ret, 0);
}
