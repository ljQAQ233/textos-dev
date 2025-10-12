
#include <textos/mm.h>
#include <textos/task.h>
#include <textos/syscall.h>
#include <textos/user/elf.h>
#include <textos/user/exec.h>

#include <gdt.h>
#include <string.h>

static char **duparg(char *const arr[])
{
    int n = 0;
    for ( ; arr[n] ; n++) ;

    char **p = malloc(sizeof(void *) * (n+1));
    for (int i = 0 ; i < n ; i++)
        p[i] = strdup(arr[i]);

    p[n] = NULL;
    return p;
}

static void brkarg(char **arr)
{
    for (int i = 0 ; arr[i] ; i++)
        free(arr[i]);
    free(arr);
}

#define N (sizeof(long)) // alignment

static inline int align(int x)
{
    return ((x) + N - 1) & ~(N - 1);
}

// 1. count length and number of arr
// 2. copy string : arr[x] -> new
static void copyarg(void *idx[], void *new, char *const arr[], int *len, int *cnt)
{
    int l = 0, i = 0, x;
    void *p = new;
    while (arr[i])
    {
        x = align(strlen(arr[i]));
        l += x;
        if (idx)
        {
            idx[i] = p;
            strcpy(p, arr[i]);
            p += x;
        }
        i++;
    }

    if (idx)
        idx[i] = NULL;

    *len = l;
    *cnt = i;
}

typedef struct
{
    uintptr_t t, v;
} auxv_t;

#define _PUT(F, T, V) \
    if (F || !V) {            \
        sp -= sizeof(auxv_t); \
        auxv_t *a = sp;       \
        a->t = T, a->v = V; }
#define PUT(F, T, V) _PUT(F, T, (uintptr_t)(V))

static void *auxv(void *sp, exeinfo_t *exe)
{
    char *a_path = sp -= align(strlen(exe->path));
    char *a_arch = sp -= align(strlen(ARCH_STRING));
    strcpy(a_path, exe->path);
    strcpy(a_arch, ARCH_STRING);

    task_t *tsk = task_current();
    PUT(1, AT_NULL, 0);
    PUT(1, AT_PHDR, exe->a_phdr);
    PUT(1, AT_PHENT, exe->a_phent);
    PUT(1, AT_PHNUM, exe->a_phnum);
    PUT(1, AT_PAGESZ, PAGE_SIZE);
    PUT(0, AT_BASE, exe->a_base);
    PUT(1, AT_ENTRY, exe->entry);
    PUT(0, AT_NOTELF, exe->a_notelf);
    PUT(1, AT_PLATFORM, a_arch);
    PUT(1, AT_EXECFN, a_path);
    PUT(1, AT_UID, tsk->ruid);
    PUT(1, AT_EUID, tsk->euid);
    PUT(1, AT_GID, tsk->rgid);
    PUT(1, AT_EGID, tsk->egid);
    return sp;
}

// build args
static void *build(void *sp, char *const argv[], char *const envp[], exeinfo_t *exe)
{
    int len;
    int nargc, nenvc;
    void **nargv, **nenvp;
    void **str_argv, **str_envp;
    
    // count
    copyarg(NULL, NULL, envp, &len, &nenvc);
    sp -= len;
    str_envp = sp;

    copyarg(NULL, NULL, argv, &len, &nargc);
    sp -= len;
    str_argv = sp;

    int rem = 1 + nargc + 1 + nenvc + 1;
    if ((((addr_t)sp - rem * N) & 0xf) != 0)
        sp -= N;
    
    // auxv
    sp = auxv(sp, exe);
    // copy
    sp -= N * (nenvc + 1);
    nenvp = sp;
    copyarg(nenvp, str_envp, envp, &len, &nenvc);

    sp -= N * (nargc + 1);
    nargv = sp;
    copyarg(nargv, str_argv, argv, &len, &nargc);

    sp -= N * 1; // argc
    (*(long *)sp) = nargc;

    return sp;
}

static void *stack()
{
    vm_region_t vm = {
        __user_stack_bot,
        __user_stack_pages,
        PROT_READ | PROT_WRITE,
        MAP_FIXED | MAP_PRIVATE | MAP_ANON,
        0, 0, 0
    };
    return mmap_anon(&vm) + __user_stack_pages * PAGE_SIZE;
}

RETVAL(int) sys_execve(char *path, char *const argv[], char *const envp[])
{
    int errno;
    exeinfo_t info;
    char **argvk = duparg(argv),
         **envpk = duparg(envp);
    task_t *curr = task_current();
    path = strdup(path);
    /*
     * create a new pagetable. elf_load may cause #PF which will be handled with
     * the current address space and its vma. if execve fails, the oldpgt will be applied
     */
    addr_t oldpgt = curr->pgt;
    vm_space_t *oldvsp = curr->vsp;
    curr->vsp = vmm_new_space(0);
    write_cr3(curr->pgt = new_pgt());
    if ((errno = elf_load(path, &info)))
        goto fail;

    void *bp, *sp;
    sp = bp = stack();
    sp = build(sp, argvk, envpk, &info);

    curr->init.main = info.entry;
    brkarg(argvk);
    brkarg(envpk);
    clear_pgt(oldpgt);
    vmm_free_space(oldvsp);
    curr->did_exec = true;
    __asm__ volatile(
            "push %0 \n" // ss
            "push %1 \n" // rsp
            "pushq $0x200\n" // rflags
            "push %2 \n" // cs
            "push %3 \n" // rip
            : :
            "i"((USER_DATA_SEG << 3) | 3), // ss
            "m"(sp),                       // rsp
            "i"((USER_CODE_SEG << 3) | 3), // cs
            "m"(info.entry),               // rip
            "D"(sp));                      // rdi
    __asm__ volatile ("iretq");

fail:
    write_cr3(oldpgt);
    curr->pgt = oldpgt;
    curr->vsp = oldvsp;
    return errno;
}
