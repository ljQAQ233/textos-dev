#include <textos/textos.h>
#include <textos/video.h>
#include <textos/video/font.h>

void kernel_main ()
{
    font_show ('M', font_get(0), 0, 0, 0xffffffff, 0x00);
}

