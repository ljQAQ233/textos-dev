; entry point for multiboot 1
; - eax - magic
; - ebx - info

[bits 32]

extern __lbase
extern __lend
extern __lend_bss

section .multiboot

; flags:
;   - MULTIBOOT_PAGE_ALIGN
;   - MULTIBOOT_MEMORY_INFO
;   - MULTIBOOT_AOUT_KLUDGE
mb1_magic equ 0x1badb002
mb1_flags equ 0x10003
mb1_cksum equ -(mb1_magic + mb1_flags)

mb1:
    align 4
    dd mb1_magic
    dd mb1_flags
    dd mb1_cksum
    dd mb1
    dd __lbase
    dd __lend
    dd __lend_bss
    dd _mb1_start

section .boot.text

_mb1_start:
    cli
    mov edi, 0xb8000
    mov ecx, 80 * 25
    mov al, 'A'
    mov ah, 0xf
.fill:
    mov [edi], ax
    add edi, 2
    loop .fill
.hlt:
    hlt;
    jmp .hlt
