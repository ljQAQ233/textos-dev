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

#define MAX_NR_CONSOLES 7
#define MIN_NR_CONSOLES 1
#define DEF_FG_CONSOLE  MIN_NR_CONSOLES

#define ARG_MAX   16
#define ARG_RESET -1

#define TABWID 8

struct con;

struct conop
{
    int (*sgr0)(struct con *con, int *argv, u32 *bg, u32 *fg);
    void (*xyputc)(struct con *con, char c, u32 x, u32 y);
    /*
     * operations to handle pixels and characters but not
     * increase any console fileds. (only graphics operations)
     */
    void (*curtoggle)(struct con *con);
    void (*pullup)(struct con *con);
    void (*pulldn)(struct con *con);
    void (*clrch)(struct con *con, int xc, int yc); // clear character at (xc, yc)
    void (*clrrg)(struct con *con, int start,
                  int end);  // clear a range of chars in the current line
    void (*clrln)(struct con *con, int line); // clear the whole line
    void (*scrclr)(struct con *con);        // clear screen
    void (*__init)(struct con *con);
};

struct conbuf
{
    u64 *buf;
};

struct con
{
    // generic
    bool scroll;
    bool cursor;
    bool hidden;
    bool underl;
    bool strike;
    u16 cur_x; /* Cursor address X */
    u16 cur_y; /* Cursor address Y */
    u32 fg, bg;
    u16 vpm_top; /* viewport top margin */
    u16 vpm_bot; /* viewport bottom margin */
    struct conbuf conbuf;
    // csi
    int argc;
    int argv[ARG_MAX];
    int state;
    bool pmcmd;
    bool acs;
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

struct con *console_get(int idx);
void console_init(struct con *con);

void conbuf_xyputc(struct con *con, char c, u32 x, u32 y);
void conbuf_init(struct con *con);
void conbuf_activate(struct con *con);

