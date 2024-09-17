#include <textos/textos.h>
#include <textos/console.h>
#include <textos/video.h>

#include <textos/dev.h>

console_t con;

void console_clear ()
{
    con.cur_x = 0;
    con.cur_y = 0;
    
    screen_clear();
}

static int console_putc (char c)
{
    switch (c)
    {
        case '\n': // 回车,将指针移动到 (0,CurY+1)
            con.cur_x = 0;
        case '\f': // 进纸符
            if (++con.cur_y >= con.col) {
                console_clear();
            }
            return c;
        case '\r': // 将指针移动到 (0,CurY)
            con.cur_x = 0;
            return c;
        case '\b': // 将指针移动到 (CurX-1,CurY)
            con.cur_x = MAX (con.cur_x - 1, 0);
            return c;
    }

    u16 x = con.cur_x * con.font->w;
    u16 y = con.cur_y * con.font->h;

    font_show (
        c,
        con.font,
        x, y, con.fg, con.bg
        );
    
    if (++con.cur_x >= con.row) {
        if (++con.cur_y >= con.col) {
            console_clear();
        }
        con.cur_x = 0;
    }

    return c;
}

#include <irq.h>

size_t console_write (dev_t *dev, char *s, size_t len)
{
    char *p;

    UNINTR_AREA({
        for (p = s ; p && *p && len ; p++, len--)
            console_putc (*p);
    });

    return (size_t)(p - s);
}

size_t console_read (dev_t *dev, char *buf, size_t len)
{
    dev_t *kbd = dev_lookup_type(DEV_CHAR, DEV_KBD);
    return kbd->read(kbd, buf, len);
}

static dev_pri_t console = {
    .dev = &(dev_t) {
        .name = "kernel console",
        .read = (void *)console_read,
        .write = (void *)console_write,
        .type = DEV_CHAR,
        .subtype = DEV_KNCON,
    }
};

void console_init ()
{
    screen_info (&con.hor, &con.ver);

    con.cur_x = 0;
    con.cur_y = 0;

    font_t *font = font_get(0);
    con.font = font;

    con.row = con.hor / font->w;
    con.col = con.ver / font->h;

    con.bg = 0x00000000;
    con.fg = 0x00ffffff;
    
    __dev_register (&console);
}

