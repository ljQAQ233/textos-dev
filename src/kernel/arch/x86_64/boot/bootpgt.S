.globl __bootpgt

.section .boot.data

.align 0x1000

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

