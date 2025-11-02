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

    /*
     * the former .boot.text section built by nasm doesn's have
     * prot EXEC. by contrast, AT&T provides "ax" which means
     * allocatable / executable
     */
    .boot.data ALIGN(4K) :
    {
        *(.multiboot)
        *(.boot.data)
    }
    .boot.text ALIGN(4K) :
    {
        *(.boot.text)
    }

    . = ALIGN(4K);    
    . += __vbase;

    HAL(.text)   { *(.text .text.*) } 
    HAL(.rodata) { *(.rodata .rodata.*) } 
    HAL(.data)   { *(.data .data.*) } 
    HAL(.bss)
    {
        *(.bss .bss.*)
        __lend_bss = . - __vbase;
    } 

    __lend = . - __vbase;
}
