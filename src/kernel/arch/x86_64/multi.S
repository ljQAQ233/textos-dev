.extern __lbase
.extern __lend
.extern __lend_bss
.extern __preinit
.extern __multi64
.globl __bootpgt

.section .multiboot, "a"

/*
 * flags:
 *   - MULTIBOOT_PAGE_ALIGN
 *   - MULTIBOOT_MEMORY_INFO
 *   - MULTIBOOT_AOUT_KLUDGE
 */
.set mb1_magic, 0x1badb002
.set mb1_flags, 0x10003
.set mb1_cksum, -(mb1_magic + mb1_flags)

.align 4
mb1:
    .long mb1_magic
    .long mb1_flags
    .long mb1_cksum
    .long mb1
    .long __lbase
    .long __lbss
    .long __lend
    .long _mb1_start

.section .boot.data, "a"

_args:
    .long 0 /* magic */
    .long 0 /* info */

.align 0x1000
_stack:
    .space 0x1000
_stack_top:

_id_pdpt:
    .set i, 0
    .rept 512
        .quad i * (1<<30) | 0x83
        .set i, i + 1 
    .endr

_id_pml4:
__bootpgt:
    .quad _id_pdpt + 0x03
    .fill 511, 8, 0

.align 8
_gdtr:
    .word _gdt_end - _gdt - 1
    .quad _gdt

.align 8
_gdt:
    .quad 0x0000000000000000 /* NULL */
    .quad 0x00af9a000000ffff /* code64 */
    .quad 0x00cf92000000ffff /* data64 */
_gdt_end:

.section .boot.text, "ax"

/*
 * entry point for multiboot 1
 * - eax - magic
 * - ebx - info
 * SEE: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Machine-state
 */
.code32
_mb1_start:
    movl %eax, _args + 0
    movl %ebx, _args + 4

    /* enable PAE */
    movl %cr4, %eax
    orl $(1 << 5), %eax
    movl %eax, %cr4

    /* load pml4 */
    movl $_id_pml4, %eax
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

    lgdt _gdtr
    ljmp $0x8, $_long_mode

.code64
_long_mode:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* enable SSE */
    movq %cr0, %rax
    andq $~(1 << 2), %rax
    orq $(1 << 1), %rax
    movq %rax, %cr0
    movq %cr4, %rax
    orq $(3 << 9), %rax
    movq %rax, %cr4

    /* c environment */
    movl $_stack_top, %esp
    movl $_stack_top, %ebp
    movl _args + 0, %edi
    movl _args + 4, %esi
    call __multi64
1:
    ud2
    jmp 1b
