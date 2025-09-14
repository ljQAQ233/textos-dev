extern void _exit(int stat);

__attribute__((weak))
int main(int argc, const char *argv[], const char *envp[])
{
    return 0;
}

extern char **__environ;

typedef void (*fn)();
extern fn __init_array_start[];
extern fn __init_array_end[];
extern fn __fini_array_start[];
extern fn __fini_array_end[];

static void libc_init()
{
    for (fn *f = __init_array_start; f < __init_array_end; f++)
        (*f)();
}

static void libc_fini()
{
    for (fn *f = __fini_array_end - 1; f >= __fini_array_start; f--)
        (*f)();
}

static void start0(long *args)
{
    int argc = args[0];
    const char **argv = (void *)&args[1];
    const char **envp = (void *)&args[1+argc+1];

    __environ = (char **)envp;
    libc_init();
    int ret = main(argc, argv, envp);
    libc_fini();
    _exit(ret);

    asm("ud2");
}

__attribute__((weak))
__attribute__((naked))
void _start(long args)
{
    asm volatile(
        "movq %rsp, %rdi\n"
        "xorq %rbp, %rbp\n"
        "call start0"
        );
}

