#include <irq.h>
#include <textos/task.h>

void gdt_init();
void idt_init();
void acpi_init();
void apic_init();
void mm_init();
void dev_init();
void task_init();
void fbdev_init();
void keyboard_init();
void serial_init();
void console_init();
void tty_init();
void buffer_init();
void pci_init();
void ahci_init();
void ide_init();
void fpu_init();
void hpet_init();
void ktm_init();
void clock_init();
void syscall_init();
void mycpu_init();
void ap_init();
void initproc();
void initproc2();

void kernel_main ()
{
    gdt_init();
    idt_init();

    acpi_init();
    apic_init();

    mm_init();

    dev_init();
    task_init();
    fbdev_init();
    keyboard_init();
    serial_init();
    console_init();
    tty_init();
    buffer_init();
    pci_init();
    ahci_init();
    ide_init();
    fpu_init();
    hpet_init();
    ktm_init();
    clock_init();
    syscall_init();
    task_create(initproc, TC_USER | TC_TSK1);

    mycpu_init();
    ap_init();
    intr_sti();

    while (true);
}

extern void fs_init();
extern void socket_init();
extern void e1000_init();
extern void dev_initnod();

#include <textos/printk.h>

void initproc()
{
    fs_init();
    socket_init();
    e1000_init();
    dev_initnod();

    printk("\033[30;47mThis is black text on a white background\033[0m\n");
    printk("A tab\ttab~\n");
    printk("QwQ\tCiallo~\n");
    printk("Kawaii\twatashi~\n");
    printk("Kawaii\r\nwatashi~\n");

    // enter 2nd phase
    initproc2();
}
