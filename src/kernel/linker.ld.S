ENTRY(kernel_init)

/*
 * define our load address and virtual address for multiboot
 * mb loader will load our kernel at the physical address __lbase
 */
__lbase = 0x100000;
__vbase = 0xffff800000000000;

#define HAL(x) x ALIGN(4K) : AT(ADDR(x) - __vbase)

SECTIONS {
    . = __lbase;

    .boot.data ALIGN(4K) :
    {
        *(.multiboot)
        *(.boot.data)
    } :bdata
    .boot.text ALIGN(4K) :
    {
        *(.boot.text)
    } :btext

    . = ALIGN(4K);    
    . += __vbase;

    HAL(.text)   { *(.text .text.*) } :text
    HAL(.rodata) { *(.rodata .rodata.*) } :rodata
    HAL(.data)   { *(.data .data.*) } :data
    HAL(.bss)
    {
        *(.bss .bss.*)
        __lend_bss = . - __vbase;
    } :bss

    __lend = . - __vbase;
}

PHDRS {
    btext PT_LOAD FLAGS(5);  /* r-x */
    bdata PT_LOAD FLAGS(6);  /* rw- */
    text PT_LOAD FLAGS(5);   /* r-x */
    rodata PT_LOAD FLAGS(4); /* r-- */
    data PT_LOAD FLAGS(6);   /* rw- */
    bss PT_LOAD FLAGS(6);    /* rw- */
}
