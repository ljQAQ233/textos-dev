/*
 * application processor trampoline
 * this part of code will be stored in .data section, ap_init is bound
 * to copy it to a physical memory region starts at `lma`. this could
 * be configured via the first some bytes below (starts at `__apboot`).
 */
.section .data

#define lma 0x1000
#define off(x) x - __apboot
#define rel(x) lma + off(x)

.org lma
.code16
__apboot:
    .byte 0xeb, off(ap16), 0, 0
    .long lma
    .long off(apboot_end)

ap16:
    cli
    lgdtl rel(gdtr)

gdt:
    .quad 0x0
    .quad 0x00cf9a000000ffff
    .quad 0x00cf92000000ffff
gdt_end:

gdtr:
    .word 0
    .long 0

apboot_end:
