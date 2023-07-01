#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/printk.h>

extern void console_init();
extern void gdt_init();
extern void idt_init();
extern void serial_init();
extern void mm_init();

#include <textos/mm.h>

void kernel_main ()
{
    serial_init();
    console_init();

    gdt_init();
    idt_init();

    mm_init();

    vmap_map(0, 0xffff0000, 1, PE_P | PE_RW, MAP_4K);
    vmap_map(0, 0xffff1000, 1, PE_P | PE_RW, MAP_4K);
    int *p1 = (int *)0xffff0000, *p2 = (int *)0xffff1000;
    *p1 = 0x2333;
    if (*p1 == *p2)
        printk("vmap_map test passed!\n");

    while (true);
}
