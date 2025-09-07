/*
 * Lazy binding resolver for x86_64, inspired by glibc
 */

extern void *__ld_resolver(void *, int);

__attribute__((naked))
void _dl_runtime_resolve()
{
    asm __volatile__(
        // store registers
        // do stack-align
        "subq $8, %%rsp\n"
        "push %%rax\n"
        "push %%rcx\n"
        "push %%rdx\n"
        "push %%rsi\n"
        "push %%rdi\n"
        "push %%r8\n"
        "push %%r9\n"

        "movq 72(%%rsp), %%rsi\n"
        "movq 64(%%rsp), %%rdi\n"
        "call __ld_resolver\n"
        "movq %%rax, %%r11\n"

        "pop %%r9\n"
        "pop %%r8\n"
        "pop %%rdi\n"
        "pop %%rsi\n"
        "pop %%rdx\n"
        "pop %%rcx\n"
        "pop %%rax\n"

        // clear align and reloc / dlptr on stack
        "addq $8, %%rsp\n"
        "addq $8, %%rsp\n"
        "addq $8, %%rsp\n"
        "jmp *%%r11\n"
        : : :
        "rax", "rcx", "rdx",
        "rsi", "rdi", "r8",
        "r9", "r11", "memory"
    );
}
