#include <cpu.h>
#include <gdt.h>
#include <intr.h>
#include <irq.h>
#include <textos/errno.h>
#include <textos/panic.h>
#include <textos/syscall.h>
#include <textos/task.h>

typedef long (*sys_func)(long, long, long, long, long, long);

#define _(name, num) extern void sys_##name();
_SYSCALL
#undef _

#define _(name, num) [num] = sys_##name,
void *sys_handlers[] = {_SYSCALL};
#undef _

__INTR_HANDLER(syscall_handler)
{
    long ret;
    sys_func func;
    int nr = frame->rax;
    if (nr < 0 || nr > SYS_maximum || !sys_handlers[nr]) {
        ret = -ENOSYS;
        goto out;
    }

    task_current()->sframe = frame;
    func = sys_handlers[nr];

    ret = func(frame->rdi, frame->rsi, frame->rdx, //
               frame->r10, frame->r8, frame->r9);
out:
    frame->rax = ret;
}

void sys_maximum()
{
    PANIC("sys_maximum mustn't be used\n");
}

extern void msyscall_handler();

#define EFER_SCE (1 << 0)
#define SYS_STAR                             \
    (0 | ((u64)(KERN_CODE_SEG << 3) << 32) | \
     ((u64)(USER_CODE32_SEG << 3) << 48))

void syscall_init()
{
    intr_register(INT_SYSCALL, syscall_handler); // 注册中断函数
    intr_setiattr(INT_SYSCALL, true);            // 用户态可用

    write_msr(MSR_STAR, SYS_STAR);
    write_msr(MSR_LSTAR, (u64)msyscall_handler);
    write_msr(MSR_FMASK, (1 << 9)); // disable irq
    write_msr(MSR_EFER, EFER_SCE | read_msr(MSR_EFER));
}
