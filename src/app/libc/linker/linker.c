#ifdef __NEED_linker

#include <elf.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
typedef uint64_t u64;
#include <stdbool.h>
#include <../kernel/klib/list.c>
#include <../kernel/klib/hlist.c>
#include <../kernel/klib/htable.c>

#warning "this lib can only run on linux due to a lack of mmap"

struct sym
{
    char *str;
    void *ptr;
    struct hlist_node node; // linked to dl::sym
};

struct dl
{
    int flags;   // RTLD_*
    int fd;      // file fd
    void *base;  // file mapping
    size_t size; // size of file
    void *virt;  // runtime image
    void *entry;
    uintptr_t *pltgot; // GOT entries
    char *strtab;      // .dynstr
    void *dynsym;      // .dynsym
    void *jmprela;     // .rela.plt
    size_t entsym;     // .dynsym entry size 
    struct htable sym; // symbol hash table
    struct list dep;   // dependent libs
    struct list l_all;
    struct list l_glb;
    struct list l_pre;
};

struct dep
{
    struct dl *dl;
    struct list node;
};

#define DLMSG 64
static int __dllog;
static int __dllevel;
static char __dlmsg[DLMSG];
static struct list __dlall = LIST_INIT(__dlall);
static struct list __dlglb = LIST_INIT(__dlglb);
static struct list __dlpre = LIST_INIT(__dlpre);
static struct dl __dlhook;
static struct dl *__dlself;

#define dl_into(x)                  \
    __dllevel += 1;                 \
    dllog("--- next level --- \n"); \
    x;                              \
    dllog("--- last level --- \n"); \
    __dllevel -= 1;
#define dlerr(fmt, arg...) \
    sprintf(__dlmsg, fmt, ##arg)
#define dllog(fmt, arg...) if (__dllog) { \
    for (int _ = 0 ; _ < __dllevel ; _++) \
        dprintf(2, "  ");                 \
    dprintf(2, fmt, ##arg); }             \

static uint32_t hash(const char *s0)
{
	const unsigned char *s = (void *)s0;
	uint_fast32_t h = 0;
	while (*s) {
		h = 16*h + *s++;
		h ^= h>>24 & 0xf0;
	}
	return h & 0xfffffff;
}

static void addsym(struct dl *dl, char *str, void *ptr)
{
    struct sym *sym = malloc(sizeof(struct sym));
    sym->str = str;
    sym->ptr = ptr;
    htkey_t key = hash(str);
    htable_add(&dl->sym, &sym->node, key);
    dllog("addsym %s [%p]\n", str, ptr);
}

static void addhook(char *str, void *ptr)
{
    addsym(&__dlhook, str, ptr);
}

#define X(record, type, mb) \
    ((type *)((void *)(record) - (void *)&((type *)((void *)0))->mb))

static inline void *lkpsym(struct dl *dl, const char *str)
{
    struct hlist_node *hptr;
    htkey_t key = hash(str);
    HTABLE_FOREACH(hptr, &dl->sym, key)
    {
        struct sym *sym = X(hptr, struct sym, node);
        if (strcmp(sym->str, str) == 0)
            return sym->ptr;
    }
    return NULL;
}

// look up symbols recursively
static void *lkprec(struct dl *dl, const char *str)
{
    struct list *lptr;
    struct hlist_node *hptr;
    htkey_t key = hash(str);
    void *res = lkpsym(dl, str);
    if (res) return res;
    LIST_FOREACH(lptr, &dl->dep)
    {
        struct dep *dep = X(lptr, struct dep, node);
        res = lkprec(dep->dl, str);
        if (res) return res;
    }
    return NULL;
}

static void *lkpglb(const char *str)
{
    struct list *lptr;
    LIST_FOREACH(lptr, &__dlglb)
    {
        struct dl *dl = X(lptr, struct dl, l_glb);
        void *res = lkprec(dl, str);
        if (res) return res;
    }
    return NULL;
}

static void *lkppre(const char *str)
{
    struct list *lptr;
    LIST_FOREACH(lptr, &__dlpre)
    {
        struct dl *dl = X(lptr, struct dl, l_pre);
        void *res = lkprec(dl, str);
        if (res) return res;
    }
    return NULL;
}

/*
 * Find the next occurrence of the desired symbol in the search order after the current
 * object. The search order is based on the load time; libraries loaded first come first.
 */
static void *lkpnxt(struct dl *prv, const char *str)
{
    struct dl *last = prv;
    struct list *lptr;
    LIST_FOREACH(lptr, &prv->dep)
    {
        last = X(lptr, struct dep, node)->dl;
        void *res = lkprec(last, str);
        if (res) return res;
    }
    for (lptr = last->l_all.next ; lptr != &__dlall ; lptr = lptr->next)
    {
        struct dl *dl = X(lptr, struct dl, l_all);
        void *res = lkprec(dl, str);
        if (res) return res;
    }
    return NULL;
}

static struct dl *byptr(uintptr_t ptr)
{
    struct list *p;
    LIST_FOREACH(p, &__dlall)
    {
        struct dl *dl = X(p, struct dl, l_all);
        if ((uintptr_t)dl->virt <= ptr &&
            (uintptr_t)dl->virt + dl->size > ptr)
            return dl;
    }
    return NULL;
}

static void *loadlibp(const char *file, int flags);

void __init_linker()
{
    if (getenv("LD_DEBUG"))
        __dllog = 1;

    struct dl *h = &__dlhook;
    h->fd = -1,
    h->base = 0,
    h->size = 0,
    list_init(&h->dep);
    htable_init(&h->sym, 8);
    list_pushback(&__dlpre, &h->l_pre);

    addhook("dlclose", NULL);
    addhook("dlerror", dlerror);
    addhook("dlopen", dlopen);
    addhook("dlsym", dlsym);

    const char *preload = getenv("LD_PRELOAD");
    if (preload)
    {
        const char *p = preload;
        while (*p)
        {
            while (*p == ' ')
                p++;
            const char *start = p;
            while (*p && *p != ':' && *p != ' ')
                p++;
            size_t len = p - start;
            if (len)
            {
                char b[256];
                if (len >= sizeof(b))
                    len = sizeof(b) - 1;
                memcpy(b, start, len);
                b[len] = '\0';
                struct dl *h = loadlibp(b, RTLD_GLOBAL);
                const char *e = h ? "okay" : dlerror();
                dllog("-> preload %s : %s\n", b, e);
                if (h) list_pushback(&__dlpre, &h->l_pre);
            }
            while (*p == ':' || *p == ' ')
                p++;
        }
    }
}

static inline size_t getsz(int fd)
{
    off_t sz = lseek(fd, 0, SEEK_END);
    return sz;
}

static inline int dochk(struct dl *dl)
{
    void *map = dl->base;
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)map;
    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || 
        hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 ||
        hdr->e_ident[EI_MAG3] != ELFMAG3) {
        dlerr("wrong elf header");
        return -1;
    }
    if (hdr->e_machine != EM_X86_64) {
        dlerr("wrong machine type");
        return -1;
    }
    if (hdr->e_type != ET_DYN) {
        dlerr("ET_SYN needed");
        return -1;
    }
    return 0;
}

static void *dlsymc(struct dl *h, struct dl *c, const char *str);

void *__ld_resolver(void *dlptr, int reloc)
{
    struct dl *dl = dlptr;
    Elf64_Rela *r = dl->jmprela + reloc * sizeof(Elf64_Rela);
    Elf64_Sym *sym = dl->dynsym + dl->entsym * ELF64_R_SYM(r->r_info);
    char *name = dl->strtab + sym->st_name;
    void *val = dlsymc(RTLD_DEFAULT, dl, name);
    if (!val)
    {
        dllog("couldn't find %s\n", name);
        exit(1);
    }

    uintptr_t *p = dl->virt + r->r_offset;
    val += r->r_addend;
    *p = (uintptr_t)val;
    dllog("__ld_resolver(%p, %d) = %s at %p\n", dlptr, reloc, name, val);
    return val;
}

static inline int dolkp(struct dl *dl)
{
    /*
     * save symbol info for looking-up
     *   - find program header
     *   - find `PT_DYNAMIC`
     */
    void *map = dl->base;
    Elf64_Ehdr *eh = (Elf64_Ehdr *)map;
    void *pmap = map + eh->e_phoff;
    void *dmap = NULL;
    for (int i = 0 ; i < eh->e_phnum ; i++)
    {
        Elf64_Phdr *ph = pmap + i * eh->e_phentsize;
        if (ph->p_type == PT_DYNAMIC)
            dmap = map + ph->p_offset;
    }
    if (!dmap)
    {
        dlerr("can not find .dynmic");
        return -1;
    }
    
    /*
     * handle symbols
     *   - `.dynsym` - DT_DYNSYM
     *   - `.dynstr` - DT_STRTAB
     *   - `.rela.dyn` - DT_RELA
     *   - `.rela.plt` - DT_JMPREL
     *   
     *   - DT_PLTREL tells the type of plt : REL / RELA
     */
    char *strtab = NULL;
    void *dynsym = NULL;
    void *pltgot = NULL;
    void *dynrela = NULL;
    void *jmprela = NULL;
    size_t entsym = 0;
    size_t szrela = 0;
    size_t entrela = 0;
    size_t szpltrel = 0;
    Elf64_Dyn *dyn = (Elf64_Dyn *)dmap;
    for ( ; dyn->d_tag != DT_NULL ; dyn++)
    {
        if (dyn->d_tag == DT_STRTAB)
            strtab = map + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_SYMTAB)
            dynsym = map + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_PLTGOT)
            pltgot = dl->virt + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_SYMENT)
            entsym = dyn->d_un.d_val;
        if (dyn->d_tag == DT_RELA)
            dynrela = map + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_RELASZ)
            szrela = dyn->d_un.d_val;
        if (dyn->d_tag == DT_RELAENT)
            entrela = dyn->d_un.d_val;
        if (dyn->d_tag == DT_PLTREL)
            dllog("pltrel %d used\n", dyn->d_un.d_val);
        if (dyn->d_tag == DT_JMPREL)
            jmprela = map + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_PLTRELSZ)
            szpltrel = dyn->d_un.d_val;
    }
    dllog("symbols:\n");
    dllog("  strtab at %p\n", strtab);
    dllog("  dynsym at %p\n", dynsym);
    dllog("  pltgot at %p\n", pltgot);
    dllog("  rela.dyn at %p\n", dynrela);
    dllog("  rela.plt at %p\n", jmprela);
    dl->strtab = strtab;
    dl->dynsym = dynsym;
    dl->jmprela = jmprela;
    dl->entsym = entsym;
    
    dyn = (Elf64_Dyn *)dmap;
    for ( ; dyn->d_tag != DT_NULL ; dyn++)
    {
        if (dyn->d_tag == DT_NEEDED)
        {
            char *lib = strtab + dyn->d_un.d_ptr;
            dl_into(void *handle = dlopen(lib, RTLD_LAZY));
            if (!handle)
            {
                dlerr("unable to dependent lib %s", lib);
                return -1;
            }
            struct dep *dep = malloc(sizeof(struct dep));
            dep->dl = handle;
            list_pushback(&dl->dep, &dep->node);
        }
    }

    /*
     * init got entries reserved for runtime linker on linux platform.
     *   - refer to glibc: glibc/sysdeps/x86_64/dl-machine.h
     */
    if (pltgot)
    {
        extern void _dl_runtime_resolve();
        dl->pltgot = pltgot;
        dl->pltgot[1] = (uintptr_t)dl;
        dl->pltgot[2] = (uintptr_t)_dl_runtime_resolve;
        dllog("resolver registered\n");
    }

    if (dynrela)
    {
        size_t nrrela = szrela / entrela;
        for (int i = 0 ; i < nrrela ; i++)
        {
            Elf64_Rela *r = dynrela + entrela * i;
            Elf64_Sym *sym = dynsym + entsym * ELF64_R_SYM(r->r_info);
            uintptr_t *p = dl->virt + r->r_offset;
            switch (ELF64_R_TYPE(r->r_info)) {
                case R_X86_64_RELATIVE:
                    *p = (uintptr_t)dl->virt + r->r_addend;
                    break;
                case R_X86_64_64:
                case R_X86_64_GLOB_DAT:
                case R_X86_64_JUMP_SLOT: {
                    uintptr_t val;
                    if (sym->st_shndx) {
                        val = (uintptr_t)dl->virt + sym->st_value;
                    } else {
                        char *name = strtab + sym->st_name;
                        void *addr = dlsymc(RTLD_DEFAULT, dl, name);
                        if (!addr) {
                            dlerr("%s not found\n", name);
                            return -1;
                        }
                        val = (uintptr_t)addr;
                    }
                    *p = val + r->r_addend;
                    break;
                }
                case R_X86_64_COPY: {
                    char *name = strtab + sym->st_name;
                    void *src = dlsymc(RTLD_DEFAULT, dl, name);
                    if (!src) {
                        dlerr("R_COPY %s not found\n", name);
                        return -1;
                    }
                    memcpy(p, src, sym->st_size);
                    break;
                }
            }
        }
    }
    if (jmprela)
    {
        // TODO: x86_64 uses RELA only, adapt it to other cases
        size_t nrpltrel = szpltrel / sizeof(Elf64_Rela);
        for (int i = 0 ; i < nrpltrel ; i++)
        {
            Elf64_Rela *r = jmprela + sizeof(Elf64_Rela) * i;
            uintptr_t *p = dl->virt + r->r_offset;
            *p += (uintptr_t)dl->virt;
        }
    }
    
    /*
     * This library's symbols are added to the global symbol table only after this point.
     * Any dlsym lookups performed before this will not see the library's own symbols.
     * Therefore, place this registration step at the end to avoid missing or shadowed symbols.
     */
    size_t nrsym;
    nrsym = strtab - (char *)dynsym;
    nrsym /= entsym;
    for (int i = 1 ; i < nrsym ; i++)
    {
        Elf64_Sym *sym = dynsym + i * entsym;
        if (!sym->st_value)
            continue;
        if (ELF64_ST_BIND(sym->st_info) == STB_GLOBAL ||
            ELF64_ST_BIND(sym->st_info) == STB_WEAK)
            addsym(dl, strtab + sym->st_name, dl->virt + sym->st_value);
    }
    return 0;
}

#define minimum(x, y) ((x) > (y) ? (y) : (x))
#define align_up(x, y) ((y) * ((x + y - 1) / y))
#define align_dn(x, y) ((y) * (x / y))

static int domap(struct dl *dl)
{
    if ((dl->size = getsz(dl->fd)) == -1) {
        dlerr("unable to fetch sz");
        return -1;
    }
    if ((dl->base = mmap(
        NULL, dl->size,
        PROT_READ,
        MAP_PRIVATE, dl->fd, 0)) == MAP_FAILED) {
        dlerr("unable to do mmap");
        return -1;
    }
    if (dochk(dl) < 0)
        return -1;
    void *map = dl->base;
    Elf64_Ehdr *eh = (Elf64_Ehdr *)map;
    void *pmap = map + eh->e_phoff;

    /*
     * determine the range of virtual address.
     */
    int lp_segs = 0;
    int lp_okay = 0;
    uintptr_t lp_max = 0;
    uintptr_t lp_min = UINTPTR_MAX;
    uintptr_t lp_size;
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
    }
    if (lp_segs == 0) {
        dlerr("no PT_LOAD found");
        return -1;
    }

    lp_size = lp_max - lp_min;
    if ((dl->virt = mmap(
        NULL, lp_size,
        PROT_NONE,
        MAP_ANON | MAP_PRIVATE, -1, 0)) == MAP_FAILED) {
        dlerr("unable to reserve pages");
        return -1;
    }

    dllog("total %d segs\n", lp_segs);
    dllog("seg range %p ~ %p\n", dl->virt + lp_min, dl->virt + lp_max);
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
        if (fsize && mmap(dl->virt + moff, fsize,
            prot, MAP_FIXED | MAP_PRIVATE,
            dl->fd, foff) == MAP_FAILED) {
            dlerr("PT_LOAD mapping failed");
            goto err;
        }

        if (ph->p_memsz > ph->p_filesz && (prot & PROT_WRITE))
        {
            uintptr_t diff = msize - fsize;
            void *rempages = dl->virt + moff + fsize;
            if (diff && mmap(rempages, diff,
                PROT_READ | PROT_WRITE | prot,
                MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) == MAP_FAILED) {
                    dlerr(".bss mapping failed");
                    goto err;
            }
            char *zero = dl->virt + ph->p_vaddr + ph->p_filesz;
            memset(zero, 0, ph->p_memsz - ph->p_filesz);
        }
        dllog("  %p -> %p, size = %lx, prot = %x\n", foff, dl->virt + moff, msize, prot);
    }
    dl->entry = dl->virt + eh->e_entry;
    dllog("file mapped to %p\n", dl->base);
    dllog("virtual base at %p\n", dl->virt);
    dllog("entry point at %p\n", dl->entry);
    return 0;
err:
    return -1;
}

static void *loadlib(const char *path, int flags)
{
    dllog("loadlib(\"%s\", %x)\n", path, flags);

    int fd;
    struct dl *dl = NULL;
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        dlerr("unable to find lib %s", path);
        goto err;
    }

    dl = malloc(sizeof(struct dl));
    if (!dl) {
        dlerr("unable to malloc");
        goto err;
    }

    dl->flags = flags;
    dl->fd = fd;
    dl->pltgot = NULL;
    htable_init(&dl->sym, 32);
    list_init(&dl->dep);
    list_init(&dl->l_all);
    list_init(&dl->l_glb);
    list_init(&dl->l_pre);
    if (domap(dl) < 0)
        goto err;
    if (dolkp(dl) < 0)
        goto err;
    dllog("gdb ->add-symbol-file %s -o %p\n", path, dl->virt);

    return dl;

err:
    if (dl)
        free(dl);
    if (fd > 0)
        close(fd);
    return NULL;
}

static void *loadlibp(const char *file, int flags)
{
    char *env[] = {
        getenv("LD_LIBRARY_PATH"),
        "/lib:/usr/lib",
    };

    size_t s1;
    size_t s2 = strlen(file);
    for (int i = 0 ; i < 2 ; i++)
    {
        char *path = env[i];
        const char *p = path;
        const char *n = path;
        size_t l = strlen(path);
        for (;n; p = n + 1)
        {
            n = strchr(p, ':');
            s1 = n ? n - p : path + l - p;
            char b[s1 + 1 + s2 + 1];
            memcpy(b + 0, p, s1);
            b[s1] = '/';
            memcpy(b + 1 + s1, file, s2);
            b[s1 + 1 + s2] = '\0';
            if (access(b, F_OK) >= 0) {
                void *res = loadlib(b, flags);
                if (res) return res;
            }
            dllog("probing %s failed\n", b);
        }
    }
    return NULL;
}

void *dlopen(const char *file, int flags)
{
    struct dl *dl;
    if (strchr(file, '/'))
        dl = loadlib(file, flags);
    else
    {
        dl = loadlibp(file, flags);
        if (!dl)
        {
            dlerr("unable to find lib %s", file);
            return NULL;
        }
    }
    if (!dl)
        return NULL;
    list_pushback(&__dlall, &dl->l_all);
    if (flags & RTLD_GLOBAL)
        list_pushback(&__dlglb, &dl->l_glb);
    return dl;
}

/*
 * another version for dlsym. with `caller` provided
 */
static void *dlsymc(struct dl *h, struct dl *c, const char *str)
{
    if (h == RTLD_NEXT)
        return c ? lkpnxt(c, str) : NULL;
    else if (h == RTLD_DEFAULT)
    {
        void *res;
        if (__dlself && (res = lkpsym(__dlself, str)))
            return res;
        if ((res = lkppre(str)))
            return res;
        if (c && (res = lkprec(c, str)))
            return res;
        return lkpglb(str);
    }
    return lkprec(h, str);
}

void *dlsym(void *restrict handle, const char *restrict name)
{
    void *retaddr = __builtin_return_address(0);
    struct dl *caller = byptr((uintptr_t)retaddr);
    return dlsymc(handle, caller, name);
}

char *dlerror()
{
    return __dlmsg;
}

#endif
