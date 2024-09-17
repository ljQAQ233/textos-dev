#include <textos/args.h>
#include <textos/syscall.h>

#define asm __asm__

static inline long syscall(int num, ...)
{
    va_list ap;
    va_start(ap, num);
    register long a0 asm ("rax") = num;
    register long a1 asm ("rdi") = va_arg(ap, long);
    register long a2 asm ("rsi") = va_arg(ap, long);
    register long a3 asm ("rdx") = va_arg(ap, long);
    register long a4 asm ("r10") = va_arg(ap, long);
    register long a5 asm ("r8") = va_arg(ap, long);
    register long a6 asm ("r9") = va_arg(ap, long);
    va_end(ap);
    asm volatile(
        "int $0x80"
        : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6)
        : "memory");
    return a0;
}

void _start ()
{
    syscall(SYSCALL_TEST);
    syscall(SYSCALL_WRITE, 1, "Hello syscall_write!\n", 22);

    char buf[5] = "test\0";
    syscall(SYSCALL_READ, 0, buf, 4);
    syscall(SYSCALL_WRITE, 1, buf, 4);
    while (1);
}
