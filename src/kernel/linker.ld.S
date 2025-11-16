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

    HAL(.text)
    {
        __kx_start = .;
        *(.text)
        __kx_end = .; 
    }
    
    HAL(.rodata)
    {
        __kr_start = .;
        *(.rodata)
        __kr_end = .;
    }

    HAL(.data)
    {
        __kw_start = .;
        *(.data)
        __mycpu_start = .;
        *(.data.mycpu)
        __mycpu_end = .;
    }
    HAL(.bss)
    {
        __lbss = . - __vbase;
        *(.bss)
        __kw_end = .;
    }
    . = ALIGN(4K);
    __lend = . - __vbase;
}
