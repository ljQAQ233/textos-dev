#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/printk.h>
#include <textos/task.h>

#include <textos/pwm.h>

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
extern void syscall_init();

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
    
    shutdown();

    dev_init();
    // keyboard_init();
    ide_init();

    clock_init();

    task_init();
    syscall_init();
    task_create(__init_proc, TC_USER | TC_TSK1);

    /*
       就让当前的栈帧成为中断栈了!
       从用户态来的中断将加载 tss 中的 rsp
    */
    __asm__ volatile (
            "movq %%rsp, %%rdi\n"
            "call __tss_set": : : "%rdi");

    intr_sti();

    while (true);
}

#include <gdt.h>
#include <textos/user/exec.h>
#include <textos/panic.h>

extern void fs_init();

static void __init_proc()
{
    fs_init();
    
    exeinfo_t exe;
    elf_load("/init.elf", &exe);

    task_t *curr = task_current();
    __asm__ volatile (
            "push %0 \n" // ss
            "push %1 \n" // rsp
            "pushfq  \n" // rflags
            "push %2 \n" // cs
            "push %3 \n" // rip
            : :
            "i"((USER_DATA_SEG << 3) | 3), // ss
            "m"(curr->init.rbp),           // rsp
            "i"((USER_CODE_SEG << 3) | 3), // cs
            "m"(exe.entry));               // rip
    __asm__ volatile ("iretq");

    PANIC("Init exiting...\n");
}
