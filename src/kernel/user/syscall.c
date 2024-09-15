#include <irq.h>
#include <intr.h>
#include <textos/syscall.h>

extern void sys_test (int arg0, int arg1, int arg2, int arg3, int arg4, int arg5);
extern int sys_fork ();

static void *handler[] = {
    [SYSCALL_TEST] = sys_test,
    [SYSCALL_FORK] = sys_fork,
};

#include <textos/panic.h>

__INTR_HANDLER(syscall_handler)
{
    if (frame->rax >= SYSCALL_MAX)
        PANIC("syscall number was out of range!\n");

    __asm__ volatile (
            "movq %1, %%rdi\n" // arg0
            "movq %2, %%rsi\n" // arg1
            "movq %3, %%rdx\n" // arg2
            "movq %4, %%rcx\n" // arg3
            "movq %5, %%r8 \n" // arg4
            "movq %6, %%r9 \n" // arg5
            "callq *%%rax\n"   // handler
            "movq %%rax, %0"
            : "=a"(frame->rax)
            : "m"(frame->rdi), "m"(frame->rsi), "m"(frame->rdx), "m"(frame->r10), "m"(frame->r8), "m"(frame->r9),
              "a"(handler[frame->rax])                   // 存放处理函数
            : "%rdi", "%rsi", "%rdx", "r10", "r8", "r9"); // 如果不告诉 gcc 我们改变了哪些寄存器, 它就有可能用这些寄存器进行寻址...
}

void syscall_init ()
{
    intr_register (INT_SYSCALL, syscall_handler); // 注册中断函数
    intr_setiattr (INT_SYSCALL, true);            // 用户态可用
}

