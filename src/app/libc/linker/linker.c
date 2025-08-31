#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/mman.h>
#include <textos/user/elf.h>
typedef uint64_t u64;
#include <../kernel/klib/hlist.c>
#include <../kernel/klib/htable.c>

#warning "this lib can only run on linux due to a lack of mmap"

struct sym
{
    char *str;
    void *ptr;
    struct hlist_node node;
};

struct dl
{
    int fd;
    void *base;
    size_t size;
    uintptr_t *pltgot;
    struct htable sym;
    struct hlist_head dep;
    struct hlist_node node;
};

struct dep
{
    struct dl *dl;
    struct hlist_node node;
};

#define DLMSG 64
static int __dllog;
static char __dlmsg[DLMSG];
static struct hlist_head __dlall;
static struct hlist_head __dlglb;

#define dlerr(fmt, arg...) \
    sprintf(__dlmsg, fmt, ##arg)
#define dllog(fmt, arg...) \
    if (__dllog) dprintf(2, fmt, ##arg)

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

#define X(record, type) \
    ((type *)((void *)(record) - (void *)&((type *)((void *)0))->node))

static void *lkpsym(struct dl *dl, const char *str)
{
    struct hlist_node *ptr;
    htkey_t key = hash(str);
    HTABLE_FOREACH(ptr, &dl->sym, key)
    {
        struct sym *sym = X(ptr, struct sym);
        if (strcmp(sym->str, str) == 0)
            return sym->ptr;
    }
    return NULL;
}

static void regdl(struct dl *dl, int flg)
{
    if (flg & RTLD_GLOBAL)
        hlist_add(&__dlglb, &dl->node);
    else
        hlist_add(&__dlall, &dl->node);
}

static struct dl *byptr(uintptr_t ptr)
{
    struct hlist_head *head[] = {
        &__dlall,
        &__dlglb,
        NULL,
    };
    for (int i = 0 ; head[i] ; i++)
    {
        struct hlist_node *p;
        HLIST_FOREACH(p, head[i])
        {
            struct dl *dl = X(p, struct dl);
            if ((uintptr_t)dl->base <= ptr &&
                (uintptr_t)dl->base + dl->size > ptr)
                return dl;
        }
    }
    return NULL;
}

void __init_linker()
{
    if (getenv("LD_DEBUG"))
        __dllog = 1;
    hlist_init(&__dlall);
    hlist_init(&__dlglb);
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

static void *loadlib(const char *path, int flags);

void __ld_resolver()
{
    printf("resolver started!!!!\n");
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
     */
    char *strtab = NULL;
    void *dynsym = NULL;
    void *pltgot = NULL;
    size_t szsym = 0;
    Elf64_Dyn *dyn = (Elf64_Dyn *)dmap;
    for ( ; dyn->d_tag != DT_NULL ; dyn++)
    {
        if (dyn->d_tag == DT_STRTAB)
            strtab = map + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_SYMTAB)
            dynsym = map + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_PLTGOT)
            pltgot = map + dyn->d_un.d_ptr;
        if (dyn->d_tag == DT_SYMENT)
            szsym = dyn->d_un.d_val;
    }
    
    dyn = (Elf64_Dyn *)dmap;
    for ( ; dyn->d_tag != DT_NULL ; dyn++)
    {
        if (dyn->d_tag == DT_NEEDED)
        {
            char *lib = strtab + dyn->d_un.d_ptr;
            void *handle = dlopen(lib, RTLD_LAZY);
            if (!handle)
            {
                dlerr("unable to dependent lib %s", lib);
                return -1;
            }
            struct dep *dep = malloc(sizeof(struct dep));
            dep->dl = handle;
            hlist_add(&dl->dep, &dep->node);
        }
    }
    
    size_t nrsym;
    nrsym = strtab - (char *)dynsym;
    nrsym /= szsym;
    for (int i = 1 ; i < nrsym ; i++)
    {
        Elf64_Sym *sym = dynsym + i * szsym;
        addsym(dl, strtab + sym->st_name, map + sym->st_value);
    }

    if (pltgot)
    {
        dl->pltgot = pltgot;
        dl->pltgot[0] = (uintptr_t)__ld_resolver;
    }
    return 0;
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

    dl->fd = fd;
    dl->pltgot = NULL;
    htable_init(&dl->sym, 32);
    hlist_init(&dl->dep);
    if ((dl->size = getsz(fd)) == -1) {
        dlerr("unable to fetch sz");
        goto err;
    }
    if ((dl->base = mmap(
        NULL, dl->size,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        dlerr("unable to do mmap");
        goto err;
    }
    if (dochk(dl) < 0)
        goto err;
    if (dolkp(dl) < 0)
        goto err;

    return dl;

err:
    if (dl)
        free(dl);
    if (fd > 0)
        close(fd);
    return NULL;
}

static char *problib(const char *file)
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
            if (access(b, F_OK) >= 0)
                return strdup(b);
            dllog("probing %s failed\n", b);
        }
    }
    return NULL;
}

void *dlopen(const char *file, int flags)
{
    if (strchr(file, '/'))
        return loadlib(file, flags);
    
    char *real = problib(file);
    if (!real)
    {
        dlerr("unable to find lib %s", file);
        return NULL;
    }
    return loadlib(real, flags);
}

void *dlsym(void *restrict handle, const char *restrict name)
{
    struct dl *dl = handle;
    return lkpsym(dl, name);
}

char *dlerror()
{
    return __dlmsg;
}
