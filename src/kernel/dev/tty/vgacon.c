#include <textos/dev/tty/console.h>
#include <textos/mm/map.h>
#include <textos/video.h>
#include <textos/video/font.h>

#define con (*__kcon)
#define empty (' ' | (con.fg << 8) | (con.bg << 12))
struct conop __conop_vga;
static u16 *base;

#include <io.h>

static void vga_movcur(int x, int y)
{
    u16 pos = y * con.row + x;
    outb(0x3d4, 0x0f);
    outb(0x3d5, pos & 0xff);
    outb(0x3d4, 0x0e);
    outb(0x3d5, (pos >> 8) & 0xff);
}

static u16 color[2][8] = {
    {
        0,  // 黑色
        4,  // 红色
        2,  // 绿色
        6,  // 黄色
        1,  // 蓝色
        5,  // 品红
        3,  // 青色
        7,  // 白色
    },
    {
        8,  // 亮黑
        12, // 亮红
        10, // 亮绿
        14, // 亮黄
        9,  // 亮蓝
        13, // 亮品红
        11, // 亮青
        15, // 亮白
    },
};

static int sgr8(int *argv, u32 *fg, u32 *bg)
{
    bool modfg;
    u16 color;
    if (argv[0] == 38)
        modfg = true;
    else if (argv[0] == 48)
        modfg = false;
    else
        return 1;

    int ret;
    // TODO: fit color
    // 24bit true-color not supported
    // 256 colors not supported
    if (argv[1] == 2)
        ret = 5;
    else if (argv[1] == 5)
        ret = 3;
    else
        return 2;

    if (modfg)
        *fg = color;
    else
        *bg = color;
    return ret;
}

static int sgr0(int *argv, u32 *bg, u32 *fg)
{
    int v = argv[0];
    if (30 <= v && v <= 37)
        *fg = color[0][v % 10];
    else if (90 <= v && v <= 97)
        *fg = color[1][v % 10];
    else if (40 <= v && v <= 47)
        *bg = color[0][v % 10];
    else if (100 <= v && v <= 107)
        *bg = color[1][v % 10];
    else
    {
        int skip = sgr8(argv, fg, bg);
        return skip;
    }
    return 1;
}

static void xyputc(char c, u32 x, u32 y)
{
    u16 chr = c | (con.fg << 8) | (con.bg << 12);
    base[y * con.row + x] = chr;
}

static void curtoggle()
{
    outb(0x3d4, 0x0a);
    u8 state = inb(0x3d5);
    outb(0x3d5, state ^ 0x20);
    vga_movcur(con.cur_x, con.cur_y);
    DEBUGK(K_LOGK, "current cursor at (%d, %d)\n", con.cur_x, con.cur_y);
}

static void pullup()
{
    u16 *dst = base;
    u16 *src = base + con.row;
    for (int i = 0; i < con.row * (con.col - 1); i++)
        *dst++ = *src++;
    for (int i = 0; i < con.row; i++)
        *dst++ = empty;
}

static void pulldn()
{
    u16 *dst = base + con.row * (con.col - 0) - 1;
    u16 *src = base + con.row * (con.col - 1) - 1;
    for (int i = 0; i < con.row * (con.col - 1); i++)
        *dst-- = *src--;
    for (int i = 0; i < con.row; i++)
        *dst-- = empty;
}

static void clrch(int xc, int yc)
{
    if (xc >= con.row || yc >= con.col)
        return;
    base[yc * con.row + xc] = empty;
}

static void clrrg(int start, int end)
{
    for (int j = start; j < MIN(con.row, end); j++)
        base[con.cur_y * con.row + j] = empty;
}

static void clrln(int line)
{
    if (line >= con.col)
        return;
    for (int j = 0; j < con.row; j++)
        base[line * con.row + j] = empty;
}

static void scrclr()
{
    for (int i = 0; i < con.col; i++)
        for (int j = 0; j < con.row; j++)
            base[i * con.row + j] = empty;
}

static void __init_impl()
{
    con.hor = 0;
    con.ver = 0;
    con.row = 80;
    con.col = 25;
    con.font = 0;
    con.fg = con.fgdef = 0x7;
    con.bg = con.bgdef = 0x0;
    con.op = &__conop_vga;
    {
        size_t pages = DIV_ROUND_UP(con.row * con.col * 2, PAGE_SIZ);
        vmap_map(0xb8000, __kern_gfx_base, pages, PE_RW | PE_P | PTE_G);
        base = (u16 *)__kern_gfx_base;
    }

    // 禁用 blink
    inb(0x3da);
    outb(0x3c0, 0x10 | 0x20);
    u8 val = inb(0x3c1);
    val &= ~0x08;
    outb(0x3c1, val);

    vga_movcur(0, 0);
    curtoggle();
    scrclr();
}

struct conop __conop_vga = {
    sgr0,
    xyputc,
    curtoggle,
    pullup,
    pulldn,
    clrch,
    clrrg,
    clrln,
    scrclr,
    __init_impl,
};
