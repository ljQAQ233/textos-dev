#include <textos/textos.h>
#include <textos/video.h>
#include <textos/video/font.h>

/*
  Show a letter by Code.

  @retval int  The index of the bitmap according to `Font`.
*/
int font_show (u8 code, font_t *f, u64 x, u64 y, u32 fg, u32 bg)
{
    u16 siz = f->w * f->h / 8;
    u8 *base = f->base + code * siz;

    u8 src = 0;
    for (u32 yi = 0 ; yi < f->h ; yi++)
    {
        for (u32 xi = 0 ; xi < f->w ; xi++)
        {
            src = base[(yi * f->w + xi) / 8];

            if (src >> (7 - xi % 8) & 0x1) {
                pixel_put (x + xi, y + yi, fg);
            } else {
                pixel_put (x + xi, y + yi, bg);
            }
        }
    }

    return code;
}
