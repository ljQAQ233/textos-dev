__attribute__((weak))
__attribute__((naked))
void _start()
{
    asm volatile(
        "movq %rsp, %rdi\n"
        "xorq %rbp, %rbp\n"
        "call __libc_start_main"
        );
}
