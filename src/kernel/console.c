#include <textos/textos.h>
#include <textos/console.h>
#include <textos/video.h>

#include <textos/dev.h>

static console_t con;

void clrcon()
{
    con.cur_x = 0;
    con.cur_y = 0;
    screen_clear();
}

#include <string.h>

static void scrln(bool up)
{
    if (up)
        screen_pullup(con.cur_y * con.font->h, con.font->h, con.bg);
    else
        screen_pulldown(0, con.font->h, con.bg);
}

// 屏幕满了怎么办? 滚! 或者 直接干掉 ૮(˶ᵔ ᵕ ᵔ˶)ა
static void full()
{
    if (con.scroll)
    {
        scrln(true);
        con.cur_x = 0;
        con.cur_y = con.cur_y - 1;
    }
    else
        clrcon();
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
    int xe = end * con.font->w,
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

// TODO : optimize
static void clrscr(int m)
{
    u32 start, end;
    if (m == 0)
    {
        for (u16 x = con.cur_x ; x < con.row ; x++)
            clrch(x, con.cur_y);
        for (u16 y = con.cur_y + 1 ; y < con.col ; y++)
            clrln(y);
    }
    else if (m == 1)
    {
        for (u16 x = 0 ; x < con.cur_x ; x++)
            clrch(x, con.cur_y);
        for (u16 y = 0 ; y < con.cur_y ; y++)
            clrln(y);
    }
    else if (m == 2)
        screen_clear();
}

static void insln()
{
    screen_pullup(con.cur_y * con.font->h, con.font->h, con.bg);
}

// TODO
static void insch()
{
    return ;
}

static void xputc(char c, u32 x, u32 y)
{
    if (con.hidden)
        return;

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

// 将光标移动到当前行的开头
static void cr()
{
    con.cur_x = 0;
    clrln(con.cur_y);
}

// 回车
static void lf()
{
    con.cur_x = 0;
    if (++con.cur_y >= con.col)
        full();
}

// 在终端中实验是将光标移动到下一行, 而不改变横坐标
static void ff()
{
    if (++con.cur_y >= con.col)
        full();
}

static void bs()
{
    con.cur_x = MAX(con.cur_x - 1, 0);
    clrch(con.cur_x, con.cur_y);
}

#define COMMIT(x) { x; return; }
#define BRKOUT(x) { x; break; }
#define NOEXAM // not tested yet

static void nor(char c)
{
    switch (c)
    {
        case '\0': COMMIT();
        case '\a': COMMIT();
        case '\t': COMMIT();
        case '\n': COMMIT(lf());
        case '\v': COMMIT(lf());
        case '\f': COMMIT(ff());
        case '\r': COMMIT(cr());
        case '\b': COMMIT(bs());
        case '\033':
            con.state = STATE_ESC;
            return;
        default: break;
    }

    u16 x = con.cur_x * con.font->w;
    u16 y = con.cur_y * con.font->h;
    xputc(c, x, y);
    
    if (++con.cur_x >= con.row) {
        if (++con.cur_y >= con.col) {
            full();
        }
        con.cur_x = 0;
    }
}

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

enum
{
    /* X */ RESET = 0,
    /*   */ BOLD = 1,
    /*   */ FAINT = 2,
    /*   */ ITALIC = 3,
    /* X */ UNDERLINE = 4,
    /*   */ BLINK = 5,
    /* X */ REVERSE = 7,
    /* X */ HIDDEN = 8,
    /* X */ STRIKE = 9,
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

static void sgr()
{
    u32 fg = 0, bg = 0;
    bool hidden = con.hidden;
    bool underl = con.underl;
    bool strike = con.strike;

    for (int i = 0 ; i < con.argc ; i++)
    {
        int v = con.argv[i];
        switch (v)
        {
        case RESET:
            fg = FG_DEF;
            bg = BG_DEF;
            hidden = false;
            underl = false;
            strike = false;
            break;

        case UNDERLINE:
            underl = true;
            break;

        case REVERSE:
            fg = fg ^ bg;
            bg = fg ^ bg;
            fg = fg ^ bg;
            break;
        
        case HIDDEN:
            hidden = true;
            break;
        
        case STRIKE:
            strike = true;
            break;
        
        default:
            if (30 <= v && v <= 37)
                fg = color[0][v % 10];
            else if (90 <= v && v <= 97)
                fg = color[1][v % 10];
            else if (40 <= v && v <= 47)
                bg = color[0][v % 10];
            else if (100 <= v && v <= 107)
                bg = color[1][v % 10];
            else
            {
                int skip = sgr8(&con.argv[i], &fg, &bg);
                i += skip - 1;
            }

            break;
        }
    }

    con.fg = fg;
    con.bg = bg;
    con.hidden = hidden;
    con.underl = underl;
    con.strike = strike;
}

static void move(int x, int y)
{
    if (x < 0)
        con.cur_x = 0;
    if (x >= con.row)
        con.cur_x = con.row - 1;
    
    if (y < 0)
        con.cur_y = 0;
    if (y >= con.col)
        con.cur_y = con.col - 1;
}

static void moved(int dx, int dy)
{
    move(con.cur_x + dx, con.cur_y + dy);
}

static void curshow(bool show)
{
    if (show)
    {
        if (con.cursor)
        {
            u16 x = con.cur_x * con.font->w;
            u16 y = con.cur_y * con.font->h;
            block_put(x, y, x + con.font->w, y + con.font->h, con.fg);
        }
    }
    else
    {
        clrch(con.cur_x, con.cur_y);
    }
}

static void curstat(bool save)
{
    static u16 x = 0, y = 0;
    if (save)
    {
        x = con.cur_x;
        y = con.cur_y;
    }
    else
    {
        con.cur_x = x;
        con.cur_y = y;
    }
}

/*
 * 状态机
 *
 * 设计参考: https://xtermjs.org/docs/api/vtfeatures/
 */

static void esc(char c)
{
    switch (c)
    {
    case '[': COMMIT(con.state = STATE_CSI);
    default:  COMMIT(con.state = STATE_NOR);
    }
}

// 是否是 private mode modifier '?' / 直接开始接受参数
static bool csi(char c)
{
    con.state = STATE_ARG;
    con.argc = ARG_RESET;

    con.pmcmd = c == '?';
    return con.pmcmd;
}

static void cmd(char c);
static void pmcmd(char c);

static void arg(char c)
{
    if (con.argc == ARG_RESET)
    {
        for (int i = 0 ; i < ARG_MAX ; i++)
            con.argv[i] = 0;
        con.argc = 0;
    }

    if ('0' <= c && c <= '9')
        con.argv[con.argc] = con.argv[con.argc] * 10 + c - '0';
    else if (c == ';')
    {
        // drop
        if (++con.argc > ARG_MAX)
            --con.argc;
    }
    else
    {
        ++con.argc;
        if (con.pmcmd)
            pmcmd(c);
        else
            cmd(c);
    }
}

static void cmd(char c)
{
    switch (c)
    {
    case 'F': NOEXAM
        move(0, con.cur_y);
    case 'A': NOEXAM BRKOUT(moved(0, -con.argv[0]));
    case 'E': NOEXAM
        move(0, con.cur_y);
    case 'e': NOEXAM
    case 'B': NOEXAM BRKOUT(moved(0,  con.argv[0]));
    case 'a': NOEXAM
    case 'C': NOEXAM BRKOUT(moved( con.argv[0], 0));
    case 'D': NOEXAM BRKOUT(moved(-con.argv[0], 0));

    case '`': NOEXAM BRKOUT(move(con.argv[0], con.cur_y));
    case 'd': NOEXAM BRKOUT(move(con.cur_x, con.argv[0]));
    
    case 'H': NOEXAM
    case 'f': NOEXAM BRKOUT(move(con.argv[0], con.argv[1]));
    
    case 'Z': NOEXAM BRKOUT(); // tab

    case 'L': NOEXAM BRKOUT({
        while (con.argv[0]--)
            insln();
    });

    case '@': NOEXAM BRKOUT({
        while (con.argv[0]--)
            insch();
    })

    case 'M': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con.argv[0], 1) ; i++)
            clrln(con.cur_y + i);
    });

    case 'P': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con.argv[0], 1) ; i++)
            clrch(con.cur_x + i, con.cur_y);
    });

    case 'J': NOEXAM BRKOUT(clrscr(con.argv[0]));
    case 's': NOEXAM BRKOUT(curstat(true));
    case 'u': NOEXAM BRKOUT(curstat(false));
    case 'K': NOEXAM BRKOUT({
        int mode = con.argv[0];
        int start;
        int end = con.col;
        if (mode == 0)
        {
            // 从光标处到行末端
            start = con.cur_x;
            end = con.col;
        }
        else if (mode == 1)
        {
            // 从行开始到光标处
            start = 0;
            end = con.cur_x;
        }
        else if (mode == 2)
        {
            // 整行
            start = 0;
            end = con.col;
        }
        clrrg(start, end);
    });
    case 'X': NOEXAM BRKOUT({
        clrrg(con.cur_x, MAX(con.argv[0], 1));
    });
    case 'S': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con.argv[0], 1) ; i++)
            scrln(true);
    });
    case 'T': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con.argv[0], 1) ; i++)
            scrln(false);
    });

    case 'm': BRKOUT(sgr());
    
    default:
        break;
    }
    
    con.state = STATE_NOR;
}

enum
{
    PM_CURSOR = 25,
};

static void pmcmd(char c)
{
    bool h = c == 'h';
    if (con.argv[0] == PM_CURSOR)
        curshow(h);

    con.state = STATE_NOR;
}

// 投喂状态机
static void cputc(char c)
{
    switch (con.state)
    {
    case STATE_NOR: COMMIT(nor(c));
    case STATE_ESC: COMMIT(esc(c));
    case STATE_CSI:
        if (csi(c)) COMMIT();
    case STATE_ARG: COMMIT(arg(c));
    
    default:
        break;
    }
}

/*
 * 设备驱动
 */

#include <irq.h>

size_t console_write(dev_t *dev, char *s, size_t len)
{
    char *p;

    curshow(false);
    UNINTR_AREA({
        for (p = s ; p && *p && len ; p++, len--)
            cputc(*p);
    });
    curshow(true);

    return (size_t)(p - s);
}

size_t console_read(dev_t *dev, char *buf, size_t len)
{
    dev_t *kbd = dev_lookup_type(DEV_KBD, 0);
    return kbd->read(kbd, buf, len);
}

static dev_pri_t console = {
    .dev = &(dev_t) {
        .name = "console",
        .read = (void *)console_read,
        .write = (void *)console_write,
        .type = DEV_CHAR,
        .subtype = DEV_KNCON,
    }
};

void console_init()
{
    screen_info(&con.hor, &con.ver);

    con.cur_x = 0;
    con.cur_y = 0;

    font_t *font = font_get(0);
    con.font = font;

    con.row = con.hor / font->w;
    con.col = con.ver / font->h;

    con.bg = BG_DEF;
    con.fg = FG_DEF;
    con.scroll = true;  // 屏满滚屏
    con.cursor = true;  // 显示光标
    con.hidden = false; // 显示字符
    con.underl = false; // 关下划线
    con.strike = false; // 关删除线

    con.argc = ARG_RESET;
    con.state = STATE_NOR;
    
    __dev_register(&console);
}

