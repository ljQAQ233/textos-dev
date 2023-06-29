#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/dev/serial.h>
#include <textos/panic.h>

extern void console_init();
extern void gdt_init();
extern void idt_init();
extern void serial_init();

void kernel_main ()
{
    serial_init();
    console_init();

    gdt_init();
    idt_init();

    PANIC("Panic test\n");

    while (true);
}
