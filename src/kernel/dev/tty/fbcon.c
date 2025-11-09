#include <textos/video.h>
#include <textos/video/font.h>
#include <textos/dev/tty/console.h>

#define con (*__kcon)
struct conop __conop_fb;

static u32 color[2][8] = {
    {
        RGB_COLOR(0x00, 0x00, 0x00),  // 黑色
        RGB_COLOR(0xFF, 0x00, 0x00),  // 红色
        RGB_COLOR(0x00, 0xFF, 0x00),  // 绿色
        RGB_COLOR(0xFF, 0xFF, 0x00),  // 黄色
        RGB_COLOR(0x00, 0x00, 0xFF),  // 蓝色
        RGB_COLOR(0xFF, 0x00, 0xFF),  // 洋红
        RGB_COLOR(0x00, 0xFF, 0xFF),  // 青色
        RGB_COLOR(0xC0, 0xC0, 0xC0),  // 白色 (浅灰)
    },
    {
        RGB_COLOR(0x80, 0x80, 0x80),  // 亮黑色 (深灰)
        RGB_COLOR(0xFF, 0x80, 0x80),  // 亮红色
        RGB_COLOR(0x80, 0xFF, 0x80),  // 亮绿色
        RGB_COLOR(0xFF, 0xFF, 0x80),  // 亮黄色
        RGB_COLOR(0x80, 0x80, 0xFF),  // 亮蓝色
        RGB_COLOR(0xFF, 0x80, 0xFF),  // 亮洋红
        RGB_COLOR(0x80, 0xFF, 0xFF),  // 亮青色
        RGB_COLOR(0xFF, 0xFF, 0xFF)   // 亮白色
    },
};

/*
 * 爱来自 ChatGPT :)
 */
static u32 rgb256(int x)
{
    int r, g, b;
    if (x >= 0 && x <= 15)
    {
        u32 c = color[x >> 3][x & 7];
        r = (c >> 16) & 0xFF;
        g = (c >> 8) & 0xFF;
        b = c & 0xFF;
    }
    else if (x >= 16 && x <= 231)
    {
        int offset = x - 16;
        int r6 = offset / 36;
        int g6 = (offset / 6) % 6;
        int b6 = offset % 6;
        r = r6 ? r6 * 40 + 55 : 0;
        g = g6 ? g6 * 40 + 55 : 0;
        b = b6 ? b6 * 40 + 55 : 0;
    }
    else if (x >= 232 && x <= 255)
    {
        int gray = 8 + (x - 232) * 10;
        r = g = b = gray;
    }
    return RGB_COLOR(r, g, b);
}

/*
 * 处理 m 模式的第 8 中情况, 并返回处理过了的参数个数
 */
static int sgr8(int *argv, u32 *fg, u32 *bg)
{
    bool modfg;
    u32 color;

    if (argv[0] == 38)
        modfg = true;
    else if (argv[0] == 48)
        modfg = false;
    else
        return 1;

    int ret;
    if (argv[1] == 2)
    {
        // 24bit true-color
        ret = 5;
        int r = argv[2];
        int g = argv[3];
        int b = argv[4];
        color = RGB_COLOR(r, g, b);
    }
    else if (argv[1] == 5)
    {
        // 256 colors
        ret = 3;
        color = rgb256(argv[2]);
    }
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
    x *= con.font->w;
    y *= con.font->h;
    font_show(
        c,
        con.font,
        x, y, con.fg, con.bg
        );
    
    if (con.strike)
    {
        u32 xl = x;
        u32 yl = y + con.font->h / 2 - 1;
        u32 xe = x + con.font->w;
        u32 ye = y + con.font->h / 2;
        block_put(xl, yl, xe, ye, con.fg);
    }
    if (con.underl)
    {
        u32 xl = x;
        u32 yl = y + con.font->h - 1;
        u32 xe = x + con.font->w;
        u32 ye = y + con.font->h;
        block_put(xl, yl, xe, ye, con.fg);
    }
}

static u32 _cb_currender(u32 x, u32 y, u32 color, void *private)
{
    return color ^= 0x00ffffff;
}

static void curtoggle()
{
    u16 x = con.cur_x * con.font->w;
    u16 y = con.cur_y * con.font->h;
    block_transform(x, y, x + con.font->w, y + con.font->h, _cb_currender, NULL);
}

static void pullup()
{
    screen_pullup(con.cur_y * con.font->h, con.font->h, con.bg);
}

static void pulldn()
{
    screen_pulldown(0, con.font->h, con.bg);
}

static void clrch(int xc, int yc)
{
    if (xc >= con.row || yc >= con.col)
        return;
    int x = xc * con.font->w,
        y = yc * con.font->h;
    int xe = x + con.font->w,
        ye = y + con.font->h;
    block_put(x, y, xe, ye, con.bg);
}

// erase in line - range [start, end)
static void clrrg(int start, int end)
{
    int x = start * con.font->w,
        y = con.cur_y * con.font->h;
    int xe = MIN(con.col, end) * con.font->w,
        ye = y + con.font->h;
    block_put(x, y, xe, ye, con.bg);
}

static void clrln(int line)
{
    if (line >= con.col)
        return;
    int x = 0,
        y = line * con.font->h;
    int xe = con.hor,
        ye = y + con.font->h;
    block_put(x, y, xe, ye, con.bg);
}

static void scrclr()
{
    screen_clear();
}

static void __init_impl()
{
    screen_info(&con.hor, &con.ver);
    font_t *font = font_get(0);
    con.row = con.hor / font->w;
    con.col = con.ver / font->h;
    con.font = font;
    con.fg = con.fgdef = 0x00ffffff;
    con.bg = con.bgdef = 0x00000000;
    con.op = &__conop_fb;
}

struct conop __conop_fb =
{
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
