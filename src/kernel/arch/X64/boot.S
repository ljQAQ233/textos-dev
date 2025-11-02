/*
 * entry point for multiboot 1
 * - eax - magic
 * - ebx - info
 */

.code32
.extern __lbase
.extern __lend
.extern __lend_bss

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
    .long __lend
    .long __lend_bss
    .long _mb1_start

.section .boot.text, "ax"

_mb1_start:
    cli
    movl $0xb8000, %edi
    movl $80*25, %ecx
    movb $'A', %al
    movb $0x0f, %ah
.show:
    movw %ax, (%edi)
    addl $2, %edi
    loop .show
.hlt:
    hlt
    jmp .hlt
