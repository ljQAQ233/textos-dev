.extern syscall_handler
.extern __handle_signal
.extern current_istk
.extern current_oldrsp
.extern task_stime_enter
.extern task_stime_exit

.globl msyscall_exit
.globl msyscall_handler

msyscall_handler:
    movq %rsp, %gs:current_oldsp(%rip)
    movq %gs:current_istk(%rip), %rsp

    pushq $0x23 /* user ss */
    pushq %gs:current_oldsp(%rip) /* rsp */
    pushq %r11  /* rflags */
    pushq $0x2b /* user cs */
    pushq %rcx  /* rip */
    pushq $2333 /* errcode */
    pushq $0x81 /* vector */

    pushq %rax
    pushq %rbx
    pushq $0 /* old rip */
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq $0 /* old eflags */
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    call task_stime_enter

    mov 15 * 8(%rsp), %rdi
    mov 16 * 8(%rsp), %rsi
    mov %rsp, %rdx
    call syscall_handler

msyscall_exit:
    call task_stime_exit
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    addq $8, %rsp /* skip */
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rdx
    addq $8, %rsp /* skip */
    popq %rbx
    popq %rax

    /* 涉及到 用户堆栈 的恢复, 如果发生 定时器中断
     * 则会直接使用 用户堆栈, 但是现在位于 ring0
     * 所以 并不会触发 栈切换, 索性就关掉好了
     * r11 寄存器的值在 sysret 时成为 rflags */
    cli

    addq $16, %rsp
    popq %rcx
    addq $8, %rsp
    popq %r11
    popq %rsp

    sysretq
