#include <sys/cdefs.h>

int main(int argc, const char *argv[], const char *envp[]);

__attribute__((weak))
__attribute__((naked))
void _start()
{
    asm volatile(
        "movq %rsp, %rdi\n"
        "xorq %rbp, %rbp\n"
        "leaq main(%rip), %rsi\n"
        "call __libc_start_main"
        );
}
