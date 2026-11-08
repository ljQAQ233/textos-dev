#include <textos/dev/tty/console.h>
#include <textos/klib/string.h>
#include <textos/mm/vmm.h>

/*
 * 0  ~ 23 - fg color
 * 24 ~ 31 - char
 * 32 ~ 55 - bg color
 */
#define COLOR(x)      ((u32)(x) & 0xffffff)
#define BUF_FG(elem)  COLOR((u64)elem >> 00)
#define BUF_BG(elem)  COLOR((u64)elem >> 32)
#define BUF_CHR(elem) ((char)((u64)(elem) >> 24))
#define ELEM_SIZE     sizeof(u64)

static inline u64 pack_char(const struct con *con, char c)
{
    u64 pack = 0;
    pack |= (u64)c << 24;
    pack |= (u64)con->fg << 00;
    pack |= (u64)con->bg << 32;
    return pack;
}

static inline u64 pack_empty(const struct con *con)
{
    return pack_char(con, ' ');
}

void conbuf_xyputc(struct con *con, char c, u32 x, u32 y)
{
    con->conbuf.buf[y * con->row + x] = pack_char(con, c);
    con->op->xyputc(con, c, x, y);
}

void conbuf_pullup(struct con *con)
{
    memmove(con->conbuf.buf, con->conbuf.buf + con->row,
            (con->row * con->col - con->row) * ELEM_SIZE);
    u64 empty = pack_empty(con);
    for (int j = 0; j < con->row; j++)
        con->conbuf.buf[con->row * (con->col - 1) + j] = empty;
    con->op->pullup(con);
}

void conbuf_pulldn(struct con *con)
{
    memmove(con->conbuf.buf + con->row, con->conbuf.buf,
            (con->row * con->col - con->row) * ELEM_SIZE);
    u64 empty = pack_empty(con);
    for (int j = 0; j < con->row; j++)
        con->conbuf.buf[con->row * 0 + j] = empty;
    con->op->pulldn(con);
}

void conbuf_clrch(struct con *con, int xc, int yc)
{
    con->conbuf.buf[yc * con->row + xc] = pack_empty(con);
    con->op->clrch(con, xc, yc);
}

void conbuf_clrrg(struct con *con, int start, int end)
{
    u64 empty = pack_empty(con);
    for (int j = start; j < MIN(con->row, end); j++)
        con->conbuf.buf[con->cur_y * con->row + j] = empty;
    con->op->clrrg(con, start, end);
}

void conbuf_clrln(struct con *con, int line)
{
    u64 empty = pack_empty(con);
    for (int j = 0; j < con->row; j++)
        con->conbuf.buf[con->cur_y * con->row + j] = empty;
    con->op->clrln(con, line);
}

void conbuf_scrclr(struct con *con)
{
    u64 empty = pack_empty(con);
    for (int i = 0; i < con->col; i++)
        for (int j = 0; j < con->row; j++)
            con->conbuf.buf[i * con->row + j] = empty;
    con->op->scrclr(con);
}

void conbuf_init(struct con *con)
{
    size_t chars = con->row * con->col;
    size_t pages = DIV_ROUND_UP(chars * ELEM_SIZE, PAGE_SIZE);
    con->conbuf.buf = vmm_allocpages(pages, PE_P | PE_RW);
}

void conbuf_activate(struct con *con)
{
    if (!con->conbuf.buf) {
        con->op->scrclr(con);
        return;
    }

    u32 fg = con->fg;
    u32 bg = con->bg;
    for (int y = 0; y < con->col; y++) {
        for (int x = 0; x < con->row; x++) {
            u64 pack = con->conbuf.buf[y * con->row + x];
            con->fg = BUF_FG(pack);
            con->bg = BUF_BG(pack);
            con->op->xyputc(con, BUF_CHR(pack), x, y);
        }
    }
    con->fg = fg;
    con->bg = bg;
}
