#include <textos/user/exec.h>
#include <textos/user/elf.h>
#include <textos/fs.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>
#include <textos/errno.h>

#include <string.h>

bool elf_check (Elf64_Ehdr *hdr)
{
    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 || hdr->e_ident[EI_MAG3] != ELFMAG3)
        return false;
    if (hdr->e_type != ET_EXEC)
        return false;
    if (hdr->e_machine != EM_X86_64)
        return false;
    if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
        return false;

    return true;
}

#define ALIGN_UP(target, base) ((base) * ((target + base - 1) / base))
#define ALIGN_DOWN(target, base) ((base) * (target / base))

int elf_load (char *path, exeinfo_t *exe)
{
    node_t *elf;
    vfs_open (NULL, &elf, path, O_READ);

    Elf64_Ehdr hdr;
    vfs_read (elf, &hdr, sizeof(hdr), 0);
    if (!elf_check(&hdr))
        return -ENOEXEC;

    int load = 0;

    size_t phdr_siz = hdr.e_phnum * hdr.e_phentsize;
    Elf64_Phdr *phdrs = malloc(phdr_siz);
    vfs_read (elf, phdrs, phdr_siz, hdr.e_phoff);

    bool overlay = false;
    for (int i = 0 ; i < hdr.e_phnum ; i++)
    {
        Elf64_Phdr *p = &phdrs[i];
        if (p->p_type != PT_LOAD)
            continue;

        addr_t map_addr = ALIGN_DOWN(p->p_vaddr, p->p_align);
        size_t map_siz = ALIGN_UP(p->p_vaddr + p->p_memsz - map_addr, p->p_align);
        if (overlay)
        {
            map_addr += p->p_align;
            map_siz  -= p->p_align;
        }
        
        size_t map_num = DIV_ROUND_UP(map_siz, PAGE_SIZ);
        u16    map_attr = PE_P | PE_RW | PE_US;

        overlay = false;
        if (i + 1 != hdr.e_phnum)
        {
            Elf64_Phdr *np = p + 1;
            addr_t nvaddr = ALIGN_DOWN(np->p_vaddr, np->p_align);
            if (map_addr + map_siz > nvaddr)
            {
                overlay = true;
            }
        }
        vmm_phyauto (map_addr, map_num, map_attr);
        memset ((void *)map_addr, 0, map_siz);
        vfs_read (elf, (void *)p->p_vaddr, p->p_filesz, p->p_offset);
        load++;
    }

    if (load == 0)
        return -ENOEXEC;

    exe->path = path;
    exe->entry = (void *)hdr.e_entry;

    return 0;
}

