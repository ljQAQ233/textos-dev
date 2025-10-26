#ifndef __JMP_ARC_H__
#define __JMP_ARC_H__

// rsp, rbp, rbx, r12, r13, r14, r15, rip
typedef unsigned long long __jmp_buf[8];

// rdi : env
#define _ASM_SETJMP               \
    "    leaq 8(%rsp), %rax\n"    \
    "    movq %rax, 0(%rdi)\n"    \
    "    movq %rbp, 8(%rdi)\n"    \
    "    movq %rbx, 16(%rdi)\n"   \
    "    movq %r12, 24(%rdi)\n"   \
    "    movq %r13, 32(%rdi)\n"   \
    "    movq %r14, 40(%rdi)\n"   \
    "    movq %r15, 48(%rdi)\n"   \
    "    movq (%rsp), %rax\n"     \
    "    movq %rax, 56(%rdi)\n"   \
    "    xor %eax, %eax\n"        \
    "    ret\n"

#define _ASM_LONGJMP            \
    "    movq 0(%rdi), %rsp\n"  \
    "    movq 8(%rdi), %rbp\n"  \
    "    movq 16(%rdi), %rbx\n" \
    "    movq 24(%rdi), %r12\n" \
    "    movq 32(%rdi), %r13\n" \
    "    movq 40(%rdi), %r14\n" \
    "    movq 48(%rdi), %r15\n" \
    "    movl %esi, %eax\n"     \
    "    test %eax, %eax\n"     \
    "    jnz 1f\n"              \
    "    movl $1, %eax\n"       \
    "1:\n"                      \
    "    jmp *56(%rdi)\n"

#endif
