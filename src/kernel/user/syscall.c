#include <irq.h>
#include <cpu.h>
#include <gdt.h>
#include <intr.h>
#include <textos/syscall.h>

extern void sys_read();
extern void sys_readdir();
extern void sys_write();
extern void sys_open();
extern void sys_close();
extern void sys_stat();
extern void sys_dup();
extern void sys_dup2();
extern void sys_pipe();
extern void sys_chdir();
extern void sys_mknod();
extern void sys_execve();
extern void sys_fork();
extern void sys_exit();
extern void sys_wait4();
extern void sys_test();
extern void sys_poweroff();
extern void sys_getpid();
extern void sys_getppid();

void *sys_handlers[] = {
    [SYS_read] = sys_read,
    [SYS_readdir] = sys_readdir,
    [SYS_write] = sys_write,
    [SYS_open] = sys_open,
    [SYS_close] = sys_close,
    [SYS_stat] = sys_stat,
    [SYS_dup] = sys_dup,
    [SYS_dup2] = sys_dup2,
    [SYS_pipe] = sys_pipe,
    [SYS_chdir] = sys_chdir,
    [SYS_mknod] = sys_mknod,
    [SYS_execve] = sys_execve,
    [SYS_fork] = sys_fork,
    [SYS_exit] = sys_exit,
    [SYS_wait4] = sys_wait4,
    [SYS_test] = sys_test,
    [SYS_poweroff] = sys_poweroff,
    [SYS_getpid] = sys_getpid,
    [SYS_getppid] = sys_getppid,
};

#include <textos/panic.h>

// todo : optimize

__INTR_HANDLER(syscall_handler)
{
    int nr = frame->rax;
    if (nr >= SYS_maxium)
        PANIC("syscall number was out of range!\n");
    else
        frame->rax = 0;

    __asm__ volatile (
            "movq %1, %%rdi\n" // arg0
            "movq %2, %%rsi\n" // arg1
            "movq %3, %%rdx\n" // arg2
            "movq %4, %%rcx\n" // arg3
            "movq %5, %%r8 \n" // arg4
            "movq %6, %%r9 \n" // arg5
            "callq *%%rax\n"   // handler
            "movq %%rax, %0"
            :"=a"(frame->rax)
            : "m"(frame->rdi), "m"(frame->rsi), "m"(frame->rdx),
              "m"(frame->r10), "m"(frame->r8),  "m"(frame->r9),
              "a"(sys_handlers[nr])    // 存放处理函数
            : "%rdi", "%rsi", "%rdx",
              "%r10", "%r8",  "%r9");   // 如果不告诉 gcc 我们改变了哪些寄存器, 它就有可能用这些寄存器进行寻址...
}

extern void msyscall_handler();

#define EFER_SCE (1 << 0)
#define SYS_STAR (0 | ((u64)(KERN_CODE_SEG << 3) << 32) | ((u64)(USER_CODE32_SEG << 3) << 48))

void syscall_init ()
{
    intr_register (INT_SYSCALL, syscall_handler); // 注册中断函数
    intr_setiattr (INT_SYSCALL, true);            // 用户态可用

    write_msr(MSR_STAR, SYS_STAR);
    write_msr(MSR_LSTAR, (u64)msyscall_handler);
    write_msr(MSR_FMASK, (1 << 9)); // disable irq
    write_msr(MSR_EFER, EFER_SCE);
}

