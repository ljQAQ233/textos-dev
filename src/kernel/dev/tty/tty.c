/*
 * teletypewriter
 * 
 * HACK: stop
 * TODO: window size
 * TODO: VMIN / VTIME
 * TODO: TCSADRAIN / TCSAFLUSH
 * TODO: linked with kbd
 * TODO: linked with console / get ttys separated (multi-tty)
 */
#include <textos/dev.h>
#include <textos/task.h>
#include <textos/errno.h>
#include <textos/ioctl.h>
#include <textos/klib/string.h>
#include <textos/dev/tty/tty.h>
#include <textos/dev/tty/kstoa.h>
#include <textos/dev/tty/tty_buffer.h>

typedef struct
{
    int stop;
    pid_t pgrp;
    pid_t iwaiter;
    pid_t owaiter;
    tty_buf_t ibuf;
    tty_buf_t obuf;
    struct termios tio;
    devst_t *output;
} tty_t;

tty_t tty1;

#define FC(x, y) (((x) & (y)) == (y))
#define FC_IFLAG(t, y) FC((t)->tio.c_iflag, y)
#define FC_OFLAG(t, y) FC((t)->tio.c_oflag, y)
#define FC_CFLAG(t, y) FC((t)->tio.c_cflag, y)
#define FC_LFLAG(t, y) FC((t)->tio.c_lflag, y)

static size_t tty_fetch_nc(tty_t *tty, char *buf, size_t len, size_t min)
{
    char c;
    size_t cnt = 0;
    while (cnt < len)
    {
        if (tty_buf_getc(&tty->ibuf, &c) < 0)
            break;
        *buf++ = (char)c;
        cnt++;
    }
    return cnt;
}

static size_t tty_fetch(tty_t *tty, char *buf, size_t len)
{
    char c;
    int flag;
    int end = 0;
    size_t cnt = 0;
    while (cnt < len && !end)
    {
        int flag = tty_buf_getc(&tty->ibuf, &c);
        if (flag < 0)
        {
            tty->iwaiter = task_current()->pid;
            task_block();
            flag = tty_buf_getc(&tty->ibuf, &c);
        }
        if (FC(flag, TTY_BF_END))
            end = 1;
        *buf++ = (char)c;
        cnt++;
    }
    return cnt;
}

static int tty_read(devst_t *dev, void *buf, size_t len)
{
    tty_t *tty = dev->pdata;
    if (FC_LFLAG(tty, ICANON))
        return tty_fetch(tty, buf, len);
    return tty_fetch_nc(tty, buf, len, tty->tio.c_cc[VMIN]);
}

static inline void tputc(tty_t *tty, char c)
{
    if (tty->stop)
    {
        if (tty_buf_putc(&tty->obuf, c) < 0)
        {
            tty->owaiter = task_current()->pid;
            task_block();
            tputc(tty, c);
        }
        return ;
    }
    tty->output->write(tty->output, &c, 1);
}

#define islower(c) ('a' <= c && c <= 'z')

static inline void opost(tty_t *tty, char c)
{
    if (FC_OFLAG(tty, OPOST))
    {
        if (FC_OFLAG(tty, OLCUC) && islower(c))
            c &= ~32;
        if (FC_OFLAG(tty, ONLCR) && c == '\n')
            tputc(tty, '\r');
    }
    tputc(tty, c);
}

void tdeliver(tty_t *tty)
{
    tty_buf_mark(&tty->ibuf, TTY_BF_END);
    if (tty->iwaiter >= 0)
    {
        task_unblock(tty->iwaiter);
        tty->iwaiter = -1;
    }
}

static void tstop(tty_t *tty, int st)
{
    if (!FC_IFLAG(tty, IXON))
        return ;
    tty->stop = st;
    /*
     * tty::obuf is not a ring buffer but merely a linear vector.
     */
    if (!st)
    {
        tty->output->write(tty->output, tty->obuf.buf, tty->obuf.tail);
        tty_buf_kill(&tty->obuf);
        if (tty->owaiter >= 0)
        {
            task_unblock(tty->owaiter);
            tty->owaiter = -1;
        }
    }
}

static void tsig(tty_t *tty, int sig)
{
    if (!FC_LFLAG(tty, NOFLSH))
        tty_buf_kill(&tty->ibuf);
    if (FC_LFLAG(tty, ISIG))
        kill(-tty->pgrp, sig);
}

/*
 * handle input data
 */
static int tty_feed(tty_t *tty, void *buf, size_t len)
{
    cc_t *cc = tty->tio.c_cc;
    char *p = buf;
    int cnt = 0;
    for ( ; len ; len--, cnt++, p++)
    {
        if (*p == '\r' && FC_IFLAG(&tty1, IGNCR))
            continue;
        if (*p == '\r' && FC_IFLAG(&tty1, ICRNL))
            *p = '\n';
        if (FC_LFLAG(tty, ICANON))
        {
            if (cc[VEOF] == *p)
            {
                /*
                 * VEOF sends EOF only if pressed at the start of an empty line (makes read() return 0).
                 * If the line buffer has data, Ctrl-D just sends the buffered data immediately without
                 * waiting for EOL. So pressing Ctrl-D repeatedly “flushes” input little by little.
                 */
                tdeliver(tty);
                continue;
            }
            if ('\n' == *p || cc[VEOL] == *p || cc[VEOL2] == *p)
            {
                if (FC_LFLAG(tty, ECHO) || FC_LFLAG(tty, ECHONL))
                    opost(tty, '\n');
                tty_buf_putc(&tty1.ibuf, '\n');
                tdeliver(tty);
                continue;
            }
            if (cc[VERASE] == *p)
            {
                if (FC_LFLAG(tty, ECHO | ECHOE))
                {
                    tputc(tty, '\b');
                    tputc(tty, ' ');
                    tputc(tty, '\b');
                }
                tty_buf_erase(&tty->ibuf);
                continue;
            }
            if (cc[VKILL] == *p)
            {
                tty_buf_kill(&tty->ibuf);
                continue;
            }
        }
        if (cc[VINTR] == *p)
        {
            tsig(tty, SIGINT);
            continue;
        }
        if (cc[VQUIT] == *p)
        {
            tsig(tty, SIGQUIT);
            continue;
        }
        if (cc[VSTART] == *p)
        {
            tstop(tty, 0);
            continue;
        }
        if (cc[VSTOP] == *p)
        {
            tstop(tty, 1);
            continue;
        }
        if (cc[VSUSP] == *p)
        {
            tsig(tty, SIGTSTP);
            continue;
        }
        tty_buf_putc(&tty1.ibuf, *p);
        opost(tty, *p);
    }
    return cnt;
}

void __tty_rx(keysym_t sym)
{
    char buf[16];
    int len = kstoa(sym, buf);
    tty_feed(&tty1, buf, len);
}

int tty_write(devst_t *dev, void *buf, size_t len)
{
    tty_t *tty = dev->pdata;
    cc_t *cc = tty->tio.c_cc;
    char *p = buf;
    int cnt = 0;
    for ( ; len ; len--, cnt++)
        opost(tty, *p++);
    return cnt;
}

int tty_ioctl(devst_t *dev, int req, void *argp)
{
    tty_t *tty = dev->pdata;
    switch (req)
    {
    case TCGETS:
        memcpy(argp, &tty->tio, sizeof(struct termios));
        break;
    case TCSETS + TCSANOW:
        memcpy(&tty->tio, argp, sizeof(struct termios));
        break;
    case TCSETS + TCSADRAIN:
    case TCSETS + TCSAFLUSH:
        return -EINVAL;
    case TIOCGPGRP:
        *(pid_t *)argp = tty->pgrp;
        break;
    case TIOCSPGRP:
        tty->pgrp = *(pid_t *)argp;
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static void init_tty(tty_t *tty, char *name)
{
    tty->stop = 0;
    tty->pgrp = 0;
    tty->iwaiter = -1;
    tty->owaiter = -1;
    tty_buf_init(&tty->ibuf);
    tty_buf_init(&tty->obuf);
    tty->tio = (struct termios) {
        .c_iflag = ICRNL | IXON,
        .c_oflag = OPOST | ONLCR,
        .c_cflag = CS8 | CREAD | CLOCAL,
        .c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK,
        .c_cc = {
            [VINTR]    = 0x03, /* Ctrl+C */
            [VQUIT]    = 0x1C, /* Ctrl+\ */
            [VERASE]   = 0x7F, /* DEL    */
            [VKILL]    = 0x15, /* Ctrl+U */
            [VEOF]     = 0x04, /* Ctrl+D */
            [VTIME]    = 0,
            [VMIN]     = 1,
            [VSTART]   = 0x11, /* Ctrl+Q */
            [VSTOP]    = 0x13, /* Ctrl+S */
            [VSUSP]    = 0x1A, /* Ctrl+Z */
        },
    };
    tty->output = dev_lookup_type(DEV_KNCON, 0);
    devst_t *dev = dev_new();
    dev->name = name;
    dev->type = DEV_CHAR;
    dev->subtype = DEV_TTY;
    dev->read = tty_read;
    dev->write = tty_write;
    dev->ioctl = tty_ioctl;
    dev->pdata = tty;
    dev_register(NULL, dev);
}

void tty_init()
{
    init_tty(&tty1, "tty1");
}
