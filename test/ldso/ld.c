int putstr(char *s)
{
    long l = 0;
    long ret = 1;
    for (char *p = s ; *p ; p++) l++;
    asm volatile (
        "movq $1, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "movq %3, %%rdx\n"
        "syscall\n"
        "movq %%rax, %0\n"
        : "=r" (ret)
        : "r" ((long)1), "r" (s), "r" (l)
        : "rax", "rdi", "rsi", "rdx"
    );
    return ret;
}

void start0(long *sp)
{
    int argc = *sp;
    char **argv = (void *)(sp+1);
    for (int i = 0 ; i < argc ; i++)
    {
        putstr(argv[i]);
        putstr("\n");
    }

    asm("movq $60, %rax\n"
        "xor %rdi, %rdi\n"
        "syscall");
}

__attribute__((naked))
void _start(long args)
{
    asm volatile(
        "movq %rsp, %rdi\n"
        "xorq %rbp, %rbp\n"
        "call start0"
        );
}
