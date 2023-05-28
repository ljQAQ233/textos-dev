ENTRY(kernel_init)

. = 0x100000;

SECTIONS {
    . += 0xFFFF800000000000;

    .text BLOCK(4K) : ALIGN(4K) {
        *(.text .text.*)
    }
    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata .rodata.*)
    }
    .data BLOCK(4K) : ALIGN(4K) {
        *(.data .data.*)
    }
    .bss BLOCK(4K) : ALIGN(4K) {
        *(.bss .bss.*)
    }
}

