#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/dev/serial.h>
#include <textos/panic.h>

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

    void *page;

    page = pmm_allocpages(1);
    pmm_freepages(page, 1);
    page = pmm_allocpages(5);
    pmm_freepages(page, 6);

    while (true);
}
