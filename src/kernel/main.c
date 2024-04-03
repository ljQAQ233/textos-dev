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
extern void dev_init();
extern void keyboard_init();
extern void ide_init();

extern void task_init();

#include <irq.h>

void kernel_main ()
{
    serial_init();
    console_init();

    gdt_init();
    idt_init();

    acpi_init();
    apic_init();

    mm_init();

    dev_init();
    // keyboard_init();
    ide_init();

    task_init();

    intr_sti();

    while (true);
}
