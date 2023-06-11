#include <textos/textos.h>
#include <textos/video.h>
#include <textos/console.h>
#include <textos/assert.h>

extern void console_init ();

void kernel_main ()
{
    console_init();

    ASSERTK(1 != 1);
}
