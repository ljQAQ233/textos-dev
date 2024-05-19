#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/printk.h>
#include <textos/task.h>

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
extern void clock_init();

extern void task_init();

static void __init_proc();

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

    clock_init();

    task_init();
    // task_create(__init_proc);

    intr_sti();

    while (true);
}

extern void fs_init();

static void __init_proc()
{
    UNINTR_AREA_START();
    fs_init();
    UNINTR_AREA_END();

    while (true)
        task_yield();
}
