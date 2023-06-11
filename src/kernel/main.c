#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/debug.h>

extern void console_init ();

void kernel_main ()
{
    console_init();
    
    DEBUGK ("Hello world : %p\n",&kernel_main);
}
