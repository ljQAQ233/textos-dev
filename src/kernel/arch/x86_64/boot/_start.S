#define ASM_FILE

#include <textos/boot/mb1.h>

.extern _mb1_start

.section .boot.text
.globl _start
_start:
    cmpl $MULTIBOOT_BOOTLOADER_MAGIC, %eax
    je _mb1_start
    jmp _efi_start
