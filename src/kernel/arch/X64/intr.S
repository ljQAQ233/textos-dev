.extern intr_handlers
.extern __handle_signal

.globl intr_exit
.globl intr_entries

.set HANDLER_MAX, 256
.set ENTRY_SIZ, 16
.set TASK_IFRAME, 0x10

/*
 * SS -> RSP (original RSP) -> RFLAGS -> CS -> RIP
 * - vector
 * - errcode if exists / reserved only (0x2333)
 */
#define ENTRY(vector, has_err) \
    1:; .if has_err == 0; \
        pushq $2333;      \
    .endif;               \
    pushq vector;         \
    jmp intr_caller;      \
    .fill ENTRY_SIZ - (. - 1b), 1, 0;

intr_caller:
    /* 保存寄存器 */
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq 15*8(%rsp), %rdi  /* vector */
    movq 16*8(%rsp), %rsi  /* errcode */
    movq %rsp, %rdx        /* intr context */

    /*
     * nasm allows us use `mov rax, sym` to load a address into register but it is
     * just for simplicity. but at&t asm is more serious. movq's immediate operand
     * is only 32-bit by default, which will be sign-extended to 64-bit. This causes
     * relocation issues when the symbol address is beyond the 32-bit signed range
     * (i.e., higher half kernel addresses like 0xffffffff80000000).
     */
    movabs $intr_handlers, %rax
    leaq (%rax, %rdi, 8), %rax
    call *(%rax)

    lea 15 * 8(%rsp), %rdi
    call __handle_signal

intr_exit:
    /* 恢复寄存器 */
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax

    addq $16, %rsp
    iretq

intr_entries:
    ENTRY($0, 0)
    ENTRY($1, 0)
    ENTRY($2, 0)
    ENTRY($3, 0)
    ENTRY($4, 0)
    ENTRY($5, 0)
    ENTRY($6, 0)
    ENTRY($7, 0)
    ENTRY($8, 1)
    ENTRY($9, 0)
    ENTRY($10, 1)
    ENTRY($11, 1)
    ENTRY($12, 1)
    ENTRY($13, 1)
    ENTRY($14, 1)
    ENTRY($15, 0)
    ENTRY($16, 0)
    ENTRY($17, 1)
    ENTRY($18, 0)
    ENTRY($19, 0)
    ENTRY($20, 0)
    ENTRY($21, 1)
    ENTRY($22, 0)
    ENTRY($23, 0)
    ENTRY($24, 0)
    ENTRY($25, 0)
    ENTRY($26, 0)
    ENTRY($27, 0)
    ENTRY($28, 0)
    ENTRY($29, 0)
    ENTRY($30, 0)
    ENTRY($31, 0)

    .set i, 32
    .rept HANDLER_MAX - 32
        ENTRY($i, 0)
        .set i, i+1
    .endr
