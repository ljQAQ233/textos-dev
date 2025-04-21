#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <textos/video/font.h>

enum
{
    STATE_NOR,
    STATE_ESC,
    STATE_CSI,
    STATE_PMF,
    STATE_ARG,
    STATE_CMD,
};

#define ARG_MAX 16
#define ARG_RESET -1

#define FG_DEF 0x00ffffff
#define BG_DEF 0x00000000

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
    bool scroll;
    bool cursor;
    bool hidden;
    bool underl;
    bool strike;

    bool oldch;
    u16 oldxy[2];

    int argc;
    int argv[ARG_MAX];
    int state;
    bool pmcmd;
};

typedef struct con console_t;

#endif
