/*
 * application processor trampoline
 * this part of code will be stored in .data section, ap_init is bound
 * to copy it to a physical memory region starts at `lma`. this could
 * be configured via the first some bytes below (starts at `__apboot`).
 */
.section .data
.extern ap_main

#define lma 0x1000
#define off(x) x - __apboot
#define rel(x) lma + off(x)

.org lma
.code16
.align 0x1000
.globl __apboot
__apboot:
    .byte 0xeb, off(ap16) - 2, 0, 0 /* jmp ap16 */
    .long lma             /* load memory addr */
    .long off(apboot_end) /* load memory size */
ppgt: .long 0x1234        /* pagetable phyaddr */
idlk: .long 0             /* __cpu_started lock */

ap16:
    cli
    mov %cs, %ax
    mov %ax, %ds

    lgdtl off(gdtr) /* segment offset of gdtr */
    mov %cr0, %eax
    or $1, %eax
    mov %eax, %cr0

    ljmpl $0x8, $rel(ap32)

.code32
ap32:
    movw $0x10, %ax
    movw %ax, %ds

    /* enable PAE */
    movl %cr4, %eax
    orl $(1 << 5), %eax
    movl %eax, %cr4

    /* load pml4 */
    movl rel(ppgt), %eax
    movl %eax, %cr3

    /* enable LME */
    movl $0xC0000080, %ecx
    rdmsr
    orl $(1 << 8), %eax
    wrmsr

    /* enable PG */
    movl %cr0, %eax
    orl $(1 << 31), %eax
    movl %eax, %cr0

    ljmpl $0x18, $rel(ap64)

.extern cpu_kstk
.extern __cpu_started
.extern __mycpu_ptrs
.extern __mycpu_start

.code64
ap64:
    movw $0x20, %ax
    movw %ax, %ds
    /* acquire id */
1:
    movl $1, %eax
    xchgl %eax, rel(idlk)
    test %eax, %eax
    jz 2f
    pause
    jmp 1b
2:
    mfence
    movabs $__cpu_started, %rax
    movl (%rax), %ebx
    addl $1, (%rax)
    /* release lock */
    movl $0, rel(idlk)
    mfence

    /* set mycpu (gsbase) */
    movabs __mycpu_ptrs, %rax
    movq (%rax, %rbx, 8), %rax
    movq %rax, %rdx
    shrq $32, %rdx
    movl $0xc0000101, %ecx
    wrmsr

    /* setup stack */
    movabs $cpu_kstk, %rax
    movq %gs:(%rax), %rsp
    movq %rsp, %rbp

    movabs $ap_main, %rax
    push %rax
    ret

gdt:
    .quad 0x0 /* null */
    .quad 0x00cf9a000000ffff /* code32 */
    .quad 0x00cf92000000ffff /* data32 */
    .quad 0x0020980000000000 /* code64 */
    .quad 0x0000920000000000 /* data64 */
gdt_end:

gdtr:
    .word gdt_end - gdt - 1
    .long rel(gdt)

apboot_end:

ap_high:
