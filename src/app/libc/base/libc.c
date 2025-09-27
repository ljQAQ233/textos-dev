#include <stdlib.h>

__attribute__((weak))
int main(int argc, const char *argv[], const char *envp[])
{
    return 0;
}

extern char **__environ;
extern const void **__auxv;

typedef void (*fn)();
extern fn __init_array_start[];
extern fn __init_array_end[];
extern fn __fini_array_start[];
extern fn __fini_array_end[];

extern void __init_stdio();
extern void __init_linker();
extern void __init_wrapper();

#include <sys/cdefs.h>
static void dummy() { };
__weak_alias(dummy, __init_stdio);
__weak_alias(dummy, __init_linker);
__weak_alias(dummy, __init_wrapper);

void __libc_start_init()
{
    __init_stdio();
    __init_linker();
    __init_wrapper();

    for (fn *f = __init_array_start; f < __init_array_end; f++)
        (*f)();
}

void __libc_exit_fini()
{
    for (fn *f = __fini_array_end - 1; f >= __fini_array_start; f--)
        (*f)();
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
    __libc_start_init();
    int ret = main(argc, argv, envp);
    exit(ret);

    asm("ud2");
}
