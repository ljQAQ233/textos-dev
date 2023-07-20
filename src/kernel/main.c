#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/printk.h>

extern void console_init();
extern void gdt_init();
extern void idt_init();
extern void serial_init();
extern void mm_init();
extern void acpi_init();
extern void apic_init();

#include <textos/mm.h>

void kernel_main ()
{
    serial_init();
    console_init();

    gdt_init();
    idt_init();

    acpi_init();
    apic_init();

    mm_init();

    while (true);
}
