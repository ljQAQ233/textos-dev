/*
 * teletypewriter
 * 
 * HACK: stop
 * TODO: TCSADRAIN / TCSAFLUSH
 * TODO: linked with kbd
 * TODO: linked with console / get ttys separated (multi-tty)
 */
#include <textos/dev.h>
#include <textos/task.h>
#include <textos/errno.h>
#include <textos/ioctl.h>
#include <textos/mm/heap.h>
#include <textos/klib/string.h>
#include <textos/dev/tty/tty.h>
#include <textos/dev/tty/kstoa.h>

tty_t ttys[8];
tty_t *fgtty;

#define FC(x, y) (((x) & (y)) == (y))
#define FC_IFLAG(t, y) FC((t)->tio.c_iflag, y)
#define FC_OFLAG(t, y) FC((t)->tio.c_oflag, y)
#define FC_CFLAG(t, y) FC((t)->tio.c_cflag, y)
#define FC_LFLAG(t, y) FC((t)->tio.c_lflag, y)

static void tty_nc_timeout(void *arg)
{
    tty_t *tty = (tty_t *)arg;
    tty_buf_mark(&tty->ibuf, TTY_BF_END);
    tty->timeout = true;
    if (tty->iwaiter >= 0)
    {
        task_unblock(tty->iwaiter);
        tty->iwaiter = -1;
    }
}

static inline void tflow(tty_t *tty, int stop)
{
    if (!FC_IFLAG(tty, IXOFF))
        return;
    if (tty->istop == stop)
        return;

    char c = stop ? 19 : 17;
    tty->istop = stop;
    // TODO
}

/*
 * Non-canonical mode: restricted by VTIME / VMIN
 * 
 * EXAM:
 *  - [x] VMIN = 0, VTIME = 0
 *        * Non-blocking read. Returns immediately with any available data,
 *        * or 0 if no data is present.
 *  - [x] VMIN > 0, VTIME = 0
 *        * Blocking read. Waits until at least VMIN bytes are available.
 *  - [x] VMIN = 0, VTIME > 0
 *        * Timed read. Waits up to VTIME * 100 ms for any data.
 *        * Returns immediately if data arrives, or 0 on timeout.
 *  - [x] VMIN > 0, VTIME > 0
 *        * Blocks until at least one byte is received.
 *        * Then starts the timer; if VTIME * 100 ms pass without new data,
 *        * returns what has been read (even if less than VMIN).
 *  - [ ] TEST CASES?
 * HINTS:
 *  - timeout must trigger a wake-up for the reading process
 *    * tty_t::timeout == 0 ? => wake up and return!!!
 * 
 * ATTENTION:
 *  - If len < cc[VTIME], then len is the real length to read
 *  - If buffer has been full and no reads performed, the excess
 *    part would be dropped (there's only a buffer implemented!)
 *  - TTY_BF_END would be set only when a timeout occurs
 *  - Only if VMIN > 0, VTIME > 0, timer would be reset as new data arrives
 */
static size_t tty_fetch_nc(tty_t *tty, char *buf, size_t len)
{
    cc_t *cc = tty->tio.c_cc;
    int reset = cc[VTIME] * cc[VMIN];
    char c;
    int flag;
    size_t cnt = 0;
    tty->reqlen = len;
    tty->timeout = false;

repeat:
    /*
     * is it not timeout? `tty_nc_timeout` has occured?
    */
    while (!tty->timeout && tty->reqlen)
    {
        flag = tty_buf_getc(&tty->ibuf, &c);
        if (flag < 0)
        {
            if (!reset && cc[VTIME])
            {
                ktimer(&tty->timer,
                    tty_nc_timeout,
                    tty, cc[VTIME] * 100);
            }
            if (cnt >= cc[VMIN])
            {
                break;
            }
            task_block();
            continue;
        }
        *buf++ = (char)c;
        cnt++, tty->reqlen--;
        if (reset && FC(flag, TTY_BF_END))
            break;
    }
    /*
     * if a timer has been set before and there has been a timeout, it must return
     * here, otherwise it must repeat it to read again until living up to cc[VMIN].
     */
    if (cc[VTIME])
    {
        if (tty->timeout)
            return cnt;
    }
    if (!tty->reqlen || cnt >= cc[VMIN])
        return cnt;
    goto repeat;
}

static size_t tty_fetch(tty_t *tty, char *buf, size_t len)
{
    char c;
    int flag;
    size_t cnt = 0;
    while (cnt < len)
    {
        int flag = tty_buf_getc(&tty->ibuf, &c);
        if (flag < 0)
        {
            tty->iwaiter = task_current()->pid;
            task_block();
            flag = tty_buf_getc(&tty->ibuf, &c);
        }
        *buf++ = (char)c;
        cnt++;
        if (FC(flag, TTY_BF_END))
            break;
    }
    return cnt;
}

static int tty_read(devst_t *dev, void *buf, size_t len)
{
    size_t cnt;
    tty_t *tty = dev->pdata;
    if (FC_LFLAG(tty, ICANON))
        cnt = tty_fetch(tty, buf, len);
    else
        cnt = tty_fetch_nc(tty, buf, len);

    if (tty_buf_rems(&tty->ibuf) >= TTY_XONSIZ)
        tflow(tty, 0);

    return cnt;
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
    tty->out(tty->data, &c, 1);
}

static inline void iputc(tty_t *tty, char c)
{
    tty_buf_t *b = &tty->ibuf;
    int rem = tty_buf_rems(b);
    if (rem)
    {
        tty_buf_putc(b, c);
        /*
         * If there has already been no space, IXOFF is enabled and this input device
         * supports software flow control, send XOFF to it.
         */
        if (!--rem)
            tflow(tty, 1);
    }
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
        tty->out(tty->data, tty->obuf.buf, tty->obuf.tail);
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
        if (*p == '\r' && FC_IFLAG(tty, IGNCR))
            continue;
        if (*p == '\r' && FC_IFLAG(tty, ICRNL))
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
                iputc(tty, '\n');
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
        iputc(tty, *p);
        if (!FC_LFLAG(tty, ICANON))
        {
            if (cc[VTIME] && cc[VMIN])
                ktimer(&tty->timer,
                    tty_nc_timeout, tty,
                    cc[VTIME] * 100);
            if ((tty->reqlen && !--tty->reqlen) ||
                tty_buf_full(&tty->ibuf))
                task_unblock(tty->iwaiter);
        }
        if (FC_LFLAG(tty, ECHO))
            opost(tty, *p);
    }
    return cnt;
}

void __tty_rxb(tty_t *tty, char *buf, size_t len)
{
    tty_feed(tty, buf, len);
}

void __tty_rx(tty_t *tty, keysym_t sym)
{
    char buf[16];
    if (!tty)
        tty = fgtty;
    int len = kstoa(sym, buf);
    tty_feed(tty, buf, len);
}

static int tty_write(devst_t *dev, void *buf, size_t len)
{
    tty_t *tty = dev->pdata;
    cc_t *cc = tty->tio.c_cc;
    char *p = buf;
    int cnt = 0;
    for ( ; len ; len--, cnt++)
        opost(tty, *p++);
    return cnt;
}

static int tty_ioctl(devst_t *dev, int req, void *argp)
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
        return tty->ctl(tty->data, req, argp);
    }
    return 0;
}

tty_t *tty_register(tty_t *tty, char *name, void *data, tty_ioctl_t ctl, tty_iosop_t in, tty_iosop_t out)
{
    if (!tty && !(tty = malloc(sizeof(tty_t))))
        return NULL;

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
    tty->data = data;
    tty->ctl = ctl;
    tty->in = in;
    tty->out = out;

    devst_t *dev = dev_new();
    dev->name = strdup(name);
    dev->type = DEV_CHAR;
    dev->subtype = DEV_TTY;
    dev->read = tty_read;
    dev->write = tty_write;
    dev->ioctl = tty_ioctl;
    dev->pdata = tty;
    dev_register(NULL, dev);
    return tty;
}

extern int console_ioctl(void *io, int req, void *argp);
extern ssize_t console_write(void *io, char *s, size_t len);

void tty_init()
{
    fgtty = tty_register(
        NULL, "tty1", NULL,
        (void *)console_ioctl,
        (void *)NULL,
        (void *)console_write);
}
