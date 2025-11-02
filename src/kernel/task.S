.globl __task_switch

__task_switch:
    pushq %rbp
    movq  %rsp, %rbp

    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq  %rsp, (%rsi)
    movq  %rdi, %rsp

    popq  %r15
    popq  %r14
    popq  %r13
    popq  %r12
    popq  %rbx

    popq  %rbp
    ret

