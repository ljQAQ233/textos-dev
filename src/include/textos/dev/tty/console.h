#pragma once

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

#define TABWID 8

struct conop
{
    int (*sgr0)(int *argv, u32 *bg, u32 *fg);
    void (*xyputc)(char c, u32 x, u32 y);
    /*
     * operations to handle pixels and characters but not
     * increase any console fileds. (only graphics operations)
     */
    void (*curtoggle)();
    void (*pullup)();
    void (*pulldn)();
    void (*clrch)(int xc, int yc);     // clear character at (xc, yc)
    void (*clrrg)(int start, int end); // clear a range of chars in the current line
    void (*clrln)(int line);           // clear the whole line
    void (*scrclr)();                  // clear screen
    void (*__init)();
};

struct con
{
    // generic
    bool scroll;
    bool cursor;
    bool hidden;
    bool underl;
    bool strike;
    u16 cur_x;  /* Cursor address X */
    u16 cur_y;  /* Cursor address Y */
    u32 fg, bg;
    // csi
    int argc;
    int argv[ARG_MAX];
    int state;
    bool pmcmd;
    // set by specific driver
    u32 hor;
    u32 ver;
    u16 row; /* how many chars in a line? */
    u16 col; /* how many chars in a column? */
    font_t *font;
    u32 fgdef, bgdef;
    struct conop *op;
};

extern struct con *__kcon;
