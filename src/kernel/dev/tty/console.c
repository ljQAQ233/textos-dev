// clang-format off
#include <textos/dev/tty/console.h>
#include <textos/video.h>

STATIC_ASSERT(MIN_NR_CONSOLES == 1, "");

struct con __cons[MAX_NR_CONSOLES + 1];
struct con *__kcon = &__cons[0];

static inline void clrcon(struct con *con)
{
    con->cur_y = 0;
    con->op->scrclr(con);
}

static inline void scrln(struct con *con, bool up)
{
    if (up)
        con->op->pullup(con);
    else
        con->op->pulldn(con);
}

// 屏幕满了怎么办? 滚! 或者 直接干掉 ૮(˶ᵔ ᵕ ᵔ˶)ა
static inline void full(struct con *con)
{
    if (con->scroll) {
        if (!con->vpm_bot && !con->vpm_top)
            scrln(con, true);
        con->cur_y = con->cur_y - 1;
    } else {
        clrcon(con);
    }
}

// TODO : optimize
static inline void clrscr(struct con *con, int m)
{
    if (m == 0) {
        for (u16 x = con->cur_x ; x < con->row ; x++)
            con->op->clrch(con, x, con->cur_y);
        for (u16 y = con->cur_y + 1 ; y < con->col ; y++)
            con->op->clrln(con, y);
    } else if (m == 1) {
        for (u16 x = 0 ; x < con->cur_x ; x++)
            con->op->clrch(con, x, con->cur_y);
        for (u16 y = 0 ; y < con->cur_y ; y++)
            con->op->clrln(con, y);
    } else if (m == 2)
        con->op->scrclr(con);
}

static inline void insln(struct con *con)
{
    con->op->pullup(con);
}

// TODO
static inline void insch(struct con *con)
{
    (void)con;
    return ;
}

// 将光标移动到当前行的开头
static inline void cr(struct con *con)
{
    con->cur_x = 0;
}

#define round_up(x, n) (((x) + (n) - 1) / (n) * (n))

// 水平制表符 tab
static inline void ht(struct con *con)
{
    con->cur_x = round_up(con->cur_x, TABWID);
    if (con->cur_x >= con->row)
        if (con->cur_y++ >= con->col)
            full(con);
}

// 回车
static inline void lf(struct con *con)
{
    if (++con->cur_y >= con->col)
        full(con);
}

// 翻页
// 这里在终端中实验是将光标移动到下一行, 而不改变横坐标
static inline void ff(struct con *con)
{
    if (++con->cur_y >= con->col)
        full(con);
}

// 退格, NOTE: 记得处理是不是自动换行的情况!!!
static inline void bs(struct con *con)
{
    con->cur_x = MAX(con->cur_x - 1, 0);
    con->op->clrch(con, con->cur_x, con->cur_y);
}

// Shift Out - Switch to an alternative character set.
static inline void so(struct con *con)
{
    con->acs = true;
}

// Shift In - Return to regular character set after Shift Out.
static inline void si(struct con *con)
{
    con->acs = false;
}

#define COMMIT(x) { x; return; }
#define BRKOUT(x) { x; break; }
#define NOEXAM // not tested yet
#define INVALID(c) DEBUGK(K_TRACE, "console gets invalid cmd - %c\n", c);

static inline void nor(struct con *con, char c)
{
    switch (c)
    {
        case '\0': COMMIT();
        case '\a': COMMIT();
        case '\t': COMMIT(ht(con));
        case '\n': COMMIT(lf(con));
        case '\v': COMMIT(lf(con));
        case '\f': COMMIT(ff(con));
        case '\r': COMMIT(cr(con));
        case '\b': COMMIT(bs(con));
        case '\x0e': COMMIT(so(con));
        case '\x0f': COMMIT(si(con));
        case '\033':
            con->state = STATE_ESC;
            return;
        default: break;
    }

    if (!con->hidden)
        conbuf_xyputc(con, c, con->cur_x, con->cur_y);
    
    if (++con->cur_x >= con->row) {
        if (++con->cur_y >= con->col) {
            full(con);
        }
        con->cur_x = 0;
    }
}

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

static inline void sgr(struct con *con)
{
    u32 fg = 0, bg = 0;
    bool hidden = con->hidden;
    bool underl = con->underl;
    bool strike = con->strike;

    for (int i = 0; i < con->argc; i++) {
        int v = con->argv[i];
        switch (v) {
        case RESET:
            fg = con->fgdef;
            bg = con->bgdef;
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
            i += con->op->sgr0(con, &con->argv[i], &bg, &fg) - 1;
            break;
        }
    }

    con->fg = fg;
    con->bg = bg;
    con->hidden = hidden;
    con->underl = underl;
    con->strike = strike;
}

static inline void move(struct con *con, int x, int y)
{
    if (x < 0) x = 0;
    if (x >= con->row) x = con->row - 1;
    con->cur_x = x;

    if (y < 0) con->cur_y = 0;
    if (y >= con->col) y = con->col - 1;
    con->cur_y = y;
}

#define clampd(d) ((d) ? (d) : 1)

static inline void moved(struct con *con, int dx, int dy)
{
    move(con, con->cur_x + dx, con->cur_y + dy);
}

/*
 * many opementations use REVERSE to make a cursor.
 * here I use color reverse to achieve it!!!
 */
static inline void curshow(struct con *con, bool show)
{
    if (con->cursor != show) {
        con->cursor = show;
        con->op->curtoggle(con);
    }
}

static inline void curstat(struct con *con, bool save)
{
    static u16 x = 0, y = 0;
    if (save) {
        x = con->cur_x;
        y = con->cur_y;
    } else {
        con->cur_x = x;
        con->cur_y = y;
    }
}

/*
 * 状态机
 *
 * 设计参考: https://xtermjs.org/docs/api/vtfeatures/
 */

static inline void esc(struct con *con, char c)
{
    switch (c) {
    case '[': COMMIT(con->state = STATE_CSI);
    default:  COMMIT(con->state = STATE_NOR);
    }
}

// 是否是 private mode modifier '?' / 直接开始接受参数
static inline bool csi(struct con *con, char c)
{
    con->state = STATE_ARG;
    con->argc = ARG_RESET;

    con->pmcmd = c == '?';
    return con->pmcmd;
}

static inline void cmd(struct con *con, char c);
static inline void pmcmd(struct con *con, char c);

static inline void arg(struct con *con, char c)
{
    if (con->argc == ARG_RESET) {
        for (int i = 0; i < ARG_MAX; i++)
            con->argv[i] = 0;
        con->argc = 0;
    }

    if ('0' <= c && c <= '9')
        con->argv[con->argc] = con->argv[con->argc] * 10 + c - '0';
    else if (c == ';') {
        // drop
        if (++con->argc > ARG_MAX) --con->argc;
    } else {
        ++con->argc;
        if (con->pmcmd)
            pmcmd(con, c);
        else
            cmd(con, c);
    }
}

static inline void cmd(struct con *con, char c)
{
    switch (c)
    {
    case 'F': NOEXAM
        move(con, 0, con->cur_y);
    case 'A': NOEXAM BRKOUT(moved(con, 0, -clampd(con->argv[0])));
    case 'E': NOEXAM
        move(con, 0, con->cur_y);
    case 'e': NOEXAM
    case 'B': NOEXAM BRKOUT(moved(con, 0, +clampd(con->argv[0])));
    case 'a': NOEXAM
    case 'C': NOEXAM BRKOUT(moved(con, +clampd(con->argv[0]), 0));
    case 'D': NOEXAM BRKOUT(moved(con, -clampd(con->argv[0]), 0));

    case 'd': NOEXAM BRKOUT(move(con, con->cur_x, con->argv[0]));
    
    case 'H': NOEXAM
    case 'f': NOEXAM BRKOUT(move(con, con->argv[1], con->argv[0]));

    case '`':
    case 'G': BRKOUT(move(con, con->argv[0], con->cur_y));
    
    case 'Z': NOEXAM BRKOUT(); // tab

    case 'L': NOEXAM BRKOUT({
        while (con->argv[0]--)
            insln(con);
    });

    case '@': NOEXAM BRKOUT({
        while (con->argv[0]--)
            insch(con);
    })

    case 'M': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con->argv[0], 1) ; i++)
            con->op->clrln(con, con->cur_y + i);
    });

    case 'P': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con->argv[0], 1) ; i++)
            con->op->clrch(con, con->cur_x + i, con->cur_y);
    });

    case 'J': NOEXAM BRKOUT(clrscr(con, con->argv[0]));
    case 's': NOEXAM BRKOUT(curstat(con, true));
    case 'u': NOEXAM BRKOUT(curstat(con, false));
    case 'K': NOEXAM BRKOUT({
        int mode = con->argv[0];
        int start;
        int end = con->col;
        if (mode == 0) {
            // 从光标处到行末端
            start = con->cur_x;
            end = con->col;
        } else if (mode == 1) {
            // 从行开始到光标处
            start = 0;
            end = con->cur_x;
        } else /* (mode == 2) */ {
            // 整行
            start = 0;
            end = con->col;
        }
        con->op->clrrg(con, start, end);
    });
    case 'X': NOEXAM BRKOUT({
        con->op->clrrg(con, con->cur_x, MAX(con->argv[0], 1));
    });
    case 'S': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con->argv[0], 1) ; i++)
            scrln(con, true);
    });
    case 'T': NOEXAM BRKOUT({
        for (int i = 0 ; i < MAX(con->argv[0], 1) ; i++)
            scrln(con, false);
    });

    case 'm': BRKOUT(sgr(con));

    // set viewport margin
    case 'r': BRKOUT({
        con->vpm_top = con->argv[0];
        con->vpm_bot = con->argv[1];
        con->cur_x = con->cur_y = 0;
    });
    // CSI Pm l - Reset mode
    // CSI ? Pm l - DEC private reset mode
    default:
        INVALID(c);
        break;
    }
    
    con->state = STATE_NOR;
}

enum
{
    PM_CURSOR = 25,
};

static inline void pmcmd(struct con *con, char c)
{
    bool h = c == 'h';
    if (con->argv[0] == PM_CURSOR)
        curshow(con, h);

    con->state = STATE_NOR;
}

// 投喂状态机
static inline void cputc(struct con *con, char c)
{
    switch (con->state)
    {
    case STATE_NOR: COMMIT(nor(con, c));
    case STATE_ESC: COMMIT(esc(con, c));
    case STATE_CSI:
        if (csi(con, c)) COMMIT();
    case STATE_ARG: COMMIT(arg(con, c));
    
    default:
        break;
    }
}

// clang-format on

/*
 * 设备驱动
 */

#include <irq.h>

ssize_t console_write(void *io, char *s, size_t len)
{
    struct con *con = io;
    char *p = s;
    curshow(con, false);
    UNINTR_AREA({
        for (; len; p++, len--)
            cputc(con, *p);
    });
    curshow(con, true);

    return (ssize_t)(p - s);
}

#include <textos/errno.h>
#include <textos/ioctl.h>

int console_ioctl(void *io, int req, void *argp)
{
    struct con *con = io;
    switch (req) {
    case TIOCGWINSZ: {
        struct winsize *win = argp;
        win->ws_row = con->col;
        win->ws_col = con->row;
        win->ws_xpixel = con->hor;
        win->ws_ypixel = con->ver;
        return 0;
    }
    case TIOCSWINSZ:
        return -EPERM;
    default:
        DEBUGK(K_TRACE, "console gets invalid ioctl req - %x\n", req);
        return -EINVAL;
    }
}

extern struct conop __conop_fb;
extern struct conop __conop_vga;

struct con *console_get(int idx)
{
    if (MIN_NR_CONSOLES <= idx && idx <= MAX_NR_CONSOLES)
        return &__cons[idx - 1];
    return NULL;
}

void console_init(struct con *con)
{
    if (con->init) return;

    struct conop *ops[] = {
        &__conop_fb,
        &__conop_vga,
    };
    ops[screen_istxt()]->__init(con);

    con->init = true;
    con->scroll = true;  // 屏满滚屏
    con->cursor = false; // 开始光标
    con->hidden = false; // 显示字符
    con->underl = false; // 关下划线
    con->strike = false; // 关删除线
    con->cur_x = 0;
    con->cur_y = 0;
    con->bg = con->bgdef;
    con->fg = con->fgdef;
    con->vpm_top = 0;
    con->vpm_bot = 0;
    con->argc = ARG_RESET;
    con->state = STATE_NOR;
    con->pmcmd = false;
    conbuf_init(con);
}

void console_switch(int idx)
{
    struct con *con = console_get(idx);
    if (!con) return;
    if (!con->init) console_init(con);
    curshow(con, false);
    conbuf_activate(con);
    curshow(con, true);
}
