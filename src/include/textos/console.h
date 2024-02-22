#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <textos/video/font.h>

struct con
{
    u32 hor;    /* HorizontalResolution */
    u32 ver;    /* VerticalResolution   */
    u16 row;    /* Row count */
    u16 col;    /* Column count */
    u16 cur_x;  /* Cursor address X */
    u16 cur_y;  /* Cursor address Y */
    u32 fg;
    u32 bg;
    font_t *font;
};

typedef struct con console_t;

void console_clear ();

#endif
