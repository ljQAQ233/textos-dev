#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>

extern void console_init ();

void kernel_main ()
{
    console_init();
    
    console_write ("Hello world!\n");
    console_write ("Hello world!\rM");
    console_write ("Hello world!\b?");
    console_write ("Hello world!\f?");
}
