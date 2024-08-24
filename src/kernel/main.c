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

    char *ptr[5];
    ptr[0] = malloc(2);
    ptr[1] = malloc(74);
    ptr[2] = malloc(25);
    ptr[3] = malloc(2333333);

    free(ptr[0]);
    free(ptr[1]);
    free(ptr[2]);
    free(ptr[3]);

    while (true);
}
