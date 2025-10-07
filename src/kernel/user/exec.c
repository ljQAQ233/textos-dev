
#include <textos/mm.h>
#include <textos/task.h>
#include <textos/syscall.h>
#include <textos/user/elf.h>
#include <textos/user/exec.h>

#include <gdt.h>
#include <string.h>

char **duparg(char *const arr[])
{
    int n = 0;
    for ( ; arr[n] ; n++) ;

    char **p = malloc(sizeof(void *) * (n+1));
    for (int i = 0 ; i < n ; i++)
        p[i] = strdup(arr[i]);

    p[n] = NULL;
    return p;
}

void brkarg(char **arr)
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
void copyarg(void *idx[], void *new, char *const arr[], int *len, int *cnt)
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

// build args
void *build(void *bp, char *const argv[], char *const envp[])
{
    int len;
    int nargc, nenvc;
    void **nargv, **nenvp;
    void **str_argv, **str_envp;
    
    // count
    copyarg(NULL, NULL, envp, &len, &nenvc);
    bp -= len;
    str_envp = bp;

    copyarg(NULL, NULL, argv, &len, &nargc);
    bp -= len;
    str_argv = bp;

    int rem = 1 + nargc + 1 + nenvc + 1;
    if ((((addr_t)bp - rem * N) & 0xf) != 0)
        bp -= N;

    // copy
    bp -= N * (nenvc + 1);
    nenvp = bp;
    copyarg(nenvp, str_envp, envp, &len, &nenvc);

    bp -= N * (nargc + 1);
    nargv = bp;
    copyarg(nargv, str_argv, argv, &len, &nargc);

    bp -= N * 1; // argc
    (*(long *)bp) = nargc;

    return bp;
}

void *stack()
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
    curr->vsp = mm_new_space(0);
    path = strdup(path);
    if ((errno = elf_load(path, &info)))
        goto fail;

    // a rude way to free the former pages...
    // if there's some operations, such as memset(), trigger #PF
    // the changes of these operations will be discarded here.
    addr_t *pml4 = (addr_t *)get_kpgt();
    for (int i = 0; i < 256; i++)
        pml4[i] &= ~PE_P;
    void *bp = stack();
    void *args = build(bp, argvk, envpk);

    curr->init.main = info.entry;
    brkarg(argvk);
    brkarg(envpk);
    curr->did_exec = true;
    __asm__ volatile(
            "push %0 \n" // ss
            "push %1 \n" // rsp
            "pushq $0x200\n" // rflags
            "push %2 \n" // cs
            "push %3 \n" // rip
            : :
            "i"((USER_DATA_SEG << 3) | 3), // ss
            "m"(args),                     // rsp
            "i"((USER_CODE_SEG << 3) | 3), // cs
            "m"(info.entry),               // rip
            "D"(args));                    // rdi
    __asm__ volatile ("iretq");

fail:
    return errno;
}
