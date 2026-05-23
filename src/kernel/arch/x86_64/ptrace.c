#include <textos/ptrace.h>

void arch_ptrace_to_regs(struct user_regs_struct *uregs,
                         struct intr_frame *regs, unsigned long *syscallno)
{
    regs->r15 = uregs->r15;
    regs->r14 = uregs->r14;
    regs->r13 = uregs->r13;
    regs->r12 = uregs->r12;
    regs->rbp = uregs->rbp;
    regs->rbx = uregs->rbx;
    regs->r11 = uregs->r11;
    regs->r10 = uregs->r10;
    regs->r9 = uregs->r9;
    regs->r8 = uregs->r8;
    regs->rax = uregs->rax;
    regs->rcx = uregs->rcx;
    regs->rdx = uregs->rdx;
    regs->rsi = uregs->rsi;
    regs->rdi = uregs->rdi;
    *syscallno = uregs->orig_rax;
    regs->rip = uregs->rip;
    regs->cs = uregs->cs;
    regs->rflags = uregs->eflags;
    regs->rsp = uregs->rsp;
    regs->ss = uregs->ss;
    // regs->fs_base = uregs->fs_base;
    // regs->gs_base = uregs->gs_base;
}

void arch_ptrace_to_uregs(struct user_regs_struct *uregs,
                          struct intr_frame *regs, unsigned long syscallno)
{
    uregs->r15 = regs->r15;
    uregs->r14 = regs->r14;
    uregs->r13 = regs->r13;
    uregs->r12 = regs->r12;
    uregs->rbp = regs->rbp;
    uregs->rbx = regs->rbx;
    uregs->r11 = regs->r11;
    uregs->r10 = regs->r10;
    uregs->r9 = regs->r9;
    uregs->r8 = regs->r8;
    uregs->rax = regs->rax;
    uregs->rcx = regs->rcx;
    uregs->rdx = regs->rdx;
    uregs->rsi = regs->rsi;
    uregs->rdi = regs->rdi;
    uregs->orig_rax = syscallno;
    uregs->rip = regs->rip;
    uregs->cs = regs->cs;
    uregs->eflags = regs->rflags;
    uregs->rsp = regs->rsp;
    uregs->ss = regs->ss;
    // uregs->fs_base = regs->fs_base;
    // uregs->gs_base = regs->gs_base;
    uregs->ds = 0;
    uregs->es = 0;
    uregs->fs = 0;
    uregs->gs = 0;
}
