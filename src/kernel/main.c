#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/printk.h>

extern void console_init ();

void kernel_main ()
{
    console_init();
    
    printk ("test format : %llx\n",-2333ULL);
}
