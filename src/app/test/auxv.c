#include <elf.h>
#include <stdio.h>
#include <stdint.h>

#define AT_LIST \
    X(AT_NULL) \
    X(AT_IGNORE) \
    X(AT_EXECFD) \
    X(AT_PHDR) \
    X(AT_PHENT) \
    X(AT_PHNUM) \
    X(AT_PAGESZ) \
    X(AT_BASE) \
    X(AT_FLAGS) \
    X(AT_ENTRY) \
    X(AT_NOTELF) \
    X(AT_UID) \
    X(AT_EUID) \
    X(AT_GID) \
    X(AT_EGID) \
    X(AT_PLATFORM) \
    X(AT_HWCAP) \
    X(AT_CLKTCK) \
    X(AT_SECURE) \
    X(AT_BASE_PLATFORM) \
    X(AT_RANDOM) \
    X(AT_HWCAP2) \
    X(AT_EXECFN) \
    X(AT_SYSINFO_EHDR) \
    X(AT_L1I_CACHESHAPE) \
    X(AT_L1D_CACHESHAPE) \
    X(AT_L2_CACHESHAPE) \
    X(AT_L3_CACHESHAPE) \
    X(AT_L1I_CACHESIZE) \
    X(AT_L1I_CACHEGEOMETRY) \
    X(AT_L1D_CACHESIZE) \
    X(AT_L1D_CACHEGEOMETRY) \
    X(AT_L2_CACHESIZE) \
    X(AT_L2_CACHEGEOMETRY) \
    X(AT_L3_CACHESIZE) \
    X(AT_L3_CACHEGEOMETRY) \
    X(AT_MINSIGSTKSZ) \

#define X(name) [name] = #name,
static char *at_names[] = {
    AT_LIST
};
#undef X

typedef struct
{
    uintptr_t t, v;
} auxv_t;

void dump(auxv_t *a)
{
    char *t = at_names[a->t];
    if (!t) t = "UNKNOWN";
    printf("%-20s: ", t);
    switch (a->t)
    {
        case AT_EXECFN:
        case AT_PLATFORM:
            printf("%s", (char *)a->v);
            break;
        default:
            printf("%lx", a->v);
    }
    printf("\n");
}

extern char **environ;

int main(int argc, char *argv[], char *envp[])
{
    char **env = environ;
    while (*env) env++;
    auxv_t *p = (void *)(env + 1);
    for ( ; p->t != AT_NULL ; p++)
        dump(p);
    return 0;
}
