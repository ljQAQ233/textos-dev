#include <stdlib.h>
#include <sys/cdefs.h>

__weak __hidden int main(
    int argc,
    const char *argv[],
    const char *envp[])
{
    return 0;
}

extern char **__environ;
extern const void **__auxv;

typedef void (*fn)();
extern void _init();
extern void _fini();
extern __weak __hidden fn __init_array_start[], __init_array_end[];
extern __weak __hidden fn __fini_array_start[], __fini_array_end[];
extern __weak __hidden fn __preinit_array_start[], __preinit_array_end[];

void __libc_start_init()
{
    for (fn *f = __preinit_array_start; f < __preinit_array_end; f++)
        (*f)();
    _init();
    for (fn *f = __init_array_start; f < __init_array_end; f++)
        (*f)();
}

void __libc_exit_fini()
{
    if (__fini_array_end)
        for (fn *f = __fini_array_end - 1; f >= __fini_array_start; f--)
            (*f)();
    _fini();
}

extern void __init_stdio();
extern void __init_linker();
extern void __init_wrapper();

static void dummy() { };
__weak_alias(dummy, __init_stdio);
__weak_alias(dummy, __init_linker);
__weak_alias(dummy, __init_wrapper);

__attribute__((constructor))
void __libc_internal_init()
{
    static int init = 0;
    if (!init)
    {
        __init_stdio();
        __init_linker();
        __init_wrapper();
        init = 1;
    }
}

void __libc_start_main(long *args)
{
    int argc = args[0];
    const char **argv = (void *)&args[1];
    const char **envp = (void *)&args[1+argc+1];
    const void **auxv = (const void **)envp;
    while (*auxv++);

    __environ = (char **)envp;
    __auxv = auxv;
    __libc_internal_init();
    __libc_start_init();
    int ret = main(argc, argv, envp);
    exit(ret);

    asm("ud2");
}
