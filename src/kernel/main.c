#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/debug.h>
#include <textos/printk.h>
#include <textos/dev/serial.h>

extern void console_init();
extern void gdt_init();
extern void idt_init();
extern void serial_init();

#include <string.h>

void kernel_main ()
{
    serial_init();
    console_init();

    gdt_init();
    idt_init();

    char *str = "Hello world!\n";
    int len = strlen (str);
    serial_write (str, len);

    printk(str);

    dprintk_set(K_LOGK | K_WARN);
    dprintk(K_LOGK, "%s", str);
    dprintk(K_WARN, "%s", str);

    while (true);
}
