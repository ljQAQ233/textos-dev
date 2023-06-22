#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>

extern void console_init();
extern void gdt_init();
extern void idt_init();

void kernel_main ()
{
    console_init();

    gdt_init();
    idt_init();

    __asm__ volatile ("int $0");

    while (true);
}
