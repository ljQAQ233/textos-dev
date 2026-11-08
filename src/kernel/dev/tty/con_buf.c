#include <textos/dev/tty/console.h>
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

void conbuf_xyputc(struct con *con, char c, u32 x, u32 y)
{
    u64 pack = 0;
    pack |= (u64)c << 24;
    pack |= (u64)con->fg << 00;
    pack |= (u64)con->bg << 32;
    con->conbuf.buf[y * con->row + x] = pack;
    con->op->xyputc(con, c, x, y);
}

void conbuf_init(struct con *con)
{
    size_t chars = con->row * con->col;
    size_t pages = DIV_ROUND_UP(chars * sizeof(u64), PAGE_SIZE);
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
