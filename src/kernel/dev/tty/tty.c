/*
 * teletypewriter
 *
 * HACK: stop
 * TODO: TCSADRAIN / TCSAFLUSH
 * TODO: linked with kbd
 * TODO: linked with console / get ttys separated (multi-tty)
 */
#define _GNU_SOURCE
#include <textos/dev.h>
#include <textos/dev/tty/cap.h>
#include <textos/dev/tty/console.h>
#include <textos/dev/tty/ksspec.h>
#include <textos/dev/tty/kstoa.h>
#include <textos/dev/tty/tty.h>
#include <textos/errno.h>
#include <textos/fs/poll.h>
#include <textos/ioctl.h>
#include <textos/klib/string.h>
#include <textos/klib/vsprintf.h>
#include <textos/mm/heap.h>
#include <textos/task.h>

tty_t *ttys[MAX_NR_CONSOLES + 1];
tty_t *fgtty;
tty_t ls;

#define FC(x, y)       (((x) & (y)) == (y))
#define FC_IFLAG(t, y) FC((t)->tio.c_iflag, y)
#define FC_OFLAG(t, y) FC((t)->tio.c_oflag, y)
#define FC_CFLAG(t, y) FC((t)->tio.c_cflag, y)
#define FC_LFLAG(t, y) FC((t)->tio.c_lflag, y)

static void unblock_and_clean(task_t **tsk)
{
    if (!*tsk) return;
    task_unblock(*tsk, 0);
    *tsk = NULL;
}

static void tty_nc_timeout(void *arg)
{
    tty_t *tty = (tty_t *)arg;
    tty_buf_mark(&tty->ibuf, TTY_BF_END);
    if (fs_poll_deliver(&tty->pollee, POLLIN)) return;
    tty->timeout = true;
    unblock_and_clean(&tty->iwaiter);
}

static inline void tflow(tty_t *tty, int stop)
{
    if (!FC_IFLAG(tty, IXOFF)) return;
    if (tty->istop == stop) return;

    char c = stop ? 19 : 17;
    tty->istop = stop;
    // TODO
    (void)c;
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
    int reason = 0;
    size_t cnt = 0;
    tty->reqlen = len;
    tty->timeout = false;

repeat:
    /*
     * is it not timeout? `tty_nc_timeout` has occured?
     */
    // TODO: make it volatile
    while (!tty->timeout && tty->reqlen) {
        flag = tty_buf_getc(&tty->ibuf, &c);
        if (flag < 0) {
            if (!reset && cc[VTIME]) {
                ktimer(&tty->timer, tty_nc_timeout, tty, cc[VTIME] * 100);
            }
            if (cnt >= cc[VMIN]) {
                break;
            }
            tty->iwaiter = task_current();
            reason = task_block(NULL, NULL, TASK_BLK, 0);
            if (reason < 0) break;
            continue;
        }
        *buf++ = (char)c;
        cnt++, tty->reqlen--;
        if (reset && FC(flag, TTY_BF_END)) break;
    }
    if (reason < 0) return reason;
    /*
     * if a timer has been set before and there has been a timeout, it must
     * return here, otherwise it must repeat it to read again until living up to
     * cc[VMIN].
     */
    if (cc[VTIME]) {
        if (tty->timeout) return cnt;
    }
    if (!tty->reqlen || cnt >= cc[VMIN]) return cnt;
    goto repeat;
}

static size_t tty_fetch(tty_t *tty, char *buf, size_t len)
{
    char c;
    int reason = 0;
    size_t cnt = 0;
    while (cnt < len) {
        int flag = tty_buf_getc(&tty->ibuf, &c);
        if (flag < 0) {
            tty->iwaiter = task_current();
            if ((reason = task_block(NULL, NULL, TASK_BLK, 0)) < 0) // intr?
                break;
            flag = tty_buf_getc(&tty->ibuf, &c);
            if (flag < 0) break;
        }
        *buf++ = (char)c;
        cnt++;
        if (FC(flag, TTY_BF_END)) break;
    }
    if (reason < 0) return reason;
    return cnt;
}

static int tty_read(devst_t *dev, void *buf, size_t len, ...)
{
    size_t cnt;
    tty_t *tty = dev->pdata;
    if (FC_LFLAG(tty, ICANON))
        cnt = tty_fetch(tty, buf, len);
    else
        cnt = tty_fetch_nc(tty, buf, len);

    if (tty_buf_rems(&tty->ibuf) >= TTY_XONSIZ) tflow(tty, 0);

    return cnt;
}

static inline void tputc(tty_t *tty, char c)
{
    if (tty->stop) {
        if (tty_buf_putc(&tty->obuf, c) < 0) {
            tty->owaiter = task_current();
            task_block(NULL, NULL, TASK_BLK, 0);
            tputc(tty, c);
        }
        return;
    }
    tty->out(tty->data, &c, 1);
}

static inline void iputc(tty_t *tty, char c)
{
    tty_buf_t *b = &tty->ibuf;
    int rem = tty_buf_rems(b);
    if (rem) {
        tty_buf_putc(b, c);
        /*
         * If there has already been no space, IXOFF is enabled and this input
         * device supports software flow control, send XOFF to it.
         */
        if (!--rem) tflow(tty, 1);
    }
}

#define islower(c) ('a' <= c && c <= 'z')

static inline void opost(tty_t *tty, char c)
{
    if (FC_OFLAG(tty, OPOST)) {
        if (FC_OFLAG(tty, OLCUC) && islower(c)) c &= ~32;
        if (FC_OFLAG(tty, ONLCR) && c == '\n') tputc(tty, '\r');
    }
    tputc(tty, c);
}

// c means canonical
static void tty_c_deliver(tty_t *tty)
{
    tty_buf_mark(&tty->ibuf, TTY_BF_END);
    unblock_and_clean(&tty->iwaiter);
    fs_poll_deliver(&tty->pollee, POLLIN);
}

static void tstop(tty_t *tty, int st)
{
    if (!FC_IFLAG(tty, IXON)) return;
    tty->stop = st;
    /*
     * tty::obuf is not a ring buffer but merely a linear vector.
     */
    if (!st) {
        tty->out(tty->data, tty->obuf.buf, tty->obuf.tail);
        tty_buf_kill(&tty->obuf);
        unblock_and_clean(&tty->owaiter);
    }
}

static void tsig(tty_t *tty, int sig)
{
    if (!FC_LFLAG(tty, NOFLSH)) tty_buf_kill(&tty->ibuf);
    if (FC_LFLAG(tty, ISIG)) fkill(-tty->pgrp, sig);
}

/*
 * handle input data
 */
static int tty_feed(tty_t *tty, void *buf, size_t len)
{
    cc_t *cc = tty->tio.c_cc;
    char *p = buf;
    int cnt = 0;
    for (; len; len--, cnt++, p++) {
        if (*p & 0x80 && FC_IFLAG(tty, ISTRIP)) *p &= 0x7f;
        if (*p == '\r' && FC_IFLAG(tty, IGNCR)) continue;
        if (*p == '\r' && FC_IFLAG(tty, ICRNL)) *p = '\n';
        if (*p == '\n' && FC_IFLAG(tty, INLCR))
            *p = '\r'; // let privilege INLCR > ICRNL
        if (FC_LFLAG(tty, ICANON)) {
            if (cc[VEOF] == *p) {
                /*
                 * VEOF sends EOF only if pressed at the start of an empty line
                 * (makes read() return 0). If the line buffer has data, Ctrl-D
                 * just sends the buffered data immediately without waiting for
                 * EOL. So pressing Ctrl-D repeatedly “flushes” input little by
                 * little.
                 */
                tty_c_deliver(tty);
                continue;
            }
            if ('\n' == *p || cc[VEOL] == *p || cc[VEOL2] == *p) {
                if (FC_LFLAG(tty, ECHO) || FC_LFLAG(tty, ECHONL))
                    opost(tty, '\n');
                iputc(tty, '\n');
                tty_c_deliver(tty);
                continue;
            }
            if (cc[VERASE] == *p) {
                if (FC_LFLAG(tty, ECHO | ECHOE)) {
                    tputc(tty, '\b');
                    tputc(tty, ' ');
                    tputc(tty, '\b');
                }
                tty_buf_erase(&tty->ibuf);
                continue;
            }
            if (cc[VKILL] == *p) {
                if (!tty_buf_chrs(&tty->ibuf)) continue;
                if (FC_LFLAG(tty, ECHO | ECHOK) ||
                    FC_LFLAG(tty, ECHO | ECHOKE)) {
                    for (int i = 0; i < tty_buf_chrs(&tty->ibuf); i++) {
                        tputc(tty, '\b');
                        tputc(tty, ' ');
                        tputc(tty, '\b');
                    }
                }
                if (FC_LFLAG(tty, ECHO | ECHOKE)) {
                    opost(tty, '\n');
                }
                tty_buf_kill(&tty->ibuf);
                continue;
            }
        }
        if (cc[VINTR] == *p) {
            tsig(tty, SIGINT);
            continue;
        }
        if (cc[VQUIT] == *p) {
            tsig(tty, SIGQUIT);
            continue;
        }
        if (cc[VSTART] == *p) {
            tstop(tty, 0);
            continue;
        }
        if (cc[VSTOP] == *p) {
            tstop(tty, 1);
            continue;
        }
        if (cc[VSUSP] == *p) {
            tsig(tty, SIGTSTP);
            continue;
        }
        iputc(tty, *p);
        if (!FC_LFLAG(tty, ICANON)) {
            // reset timer
            if (cc[VTIME] && cc[VMIN])
                ktimer(&tty->timer, tty_nc_timeout, tty, cc[VTIME] * 100);
            if (tty_buf_chrs(&tty->ibuf) >= tty->reqlen // live up to request
                || tty_buf_chrs(&tty->ibuf) >= cc[VMIN] // minimum chars
                || tty_buf_full(&tty->ibuf))            // no space to rx chars
            {
                if (!fs_poll_deliver(&tty->pollee, POLLIN))
                    unblock_and_clean(&tty->iwaiter);
            }
        }
        if (FC_LFLAG(tty, ECHO)) opost(tty, *p);
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
    if (!tty) tty = fgtty;
    int idx = ksspec_chtty(sym);
    if (idx) {
        console_switch(idx);
        return;
    }
    int len = kstoa(sym, buf);
    tty_feed(tty, buf, len);
}

static int tty__init_pctx(devst_t *dev, void **pctx)
{
    task_t *tsk = task_current();
    tty_t *ctty = dev->pdata;
    if ((addr_t)ctty <= MAX_NR_CONSOLES) {
        dev->pdata = ctty = tty_init_struct_on_console((addr_t)ctty);
    }
    if (ctty->ses_holders == 0 && tsk->sid == tsk->pid &&
        tsk->ctrltty == NULL) {
        tsk->ctrltty = dev;
        tty_t *ctty = dev->pdata;
        ctty->ses_id = tsk->sid;
        ctty->ses_holders = 1;
    }
    return 0;
}

static int tty_write(devst_t *dev, void *buf, size_t len, ...)
{
    tty_t *tty = dev->pdata;
    char *p = buf;
    int cnt = 0;
    for (; len; len--, cnt++)
        opost(tty, *p++);
    return cnt;
}

static int tty_ioctl(devst_t *dev, int req, void *argp)
{
    tty_t *tty = dev->pdata;
    switch (req) {
    case TCGETS:
        memcpy(argp, &tty->tio, sizeof(struct termios));
        break;
    case TCSETS + TCSADRAIN:
    case TCSETS + TCSAFLUSH:
    case TCSETS + TCSANOW:
        tty_cap_check_attr(argp);
        memcpy(&tty->tio, argp, sizeof(struct termios));
        DEBUGK(K_TRACE, "tty attr iflag=%o,oflag=%o,cflag=%o,lflag=%o\n",
               tty->tio.c_iflag, tty->tio.c_oflag, tty->tio.c_cflag,
               tty->tio.c_lflag);
        break;
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

int tty_poll_setup(devst_t *this, struct fs_poller *poller)
{
    tty_t *tty = this->pdata;
    if (tty_buf_chrs(&tty->ibuf) > 0)
        if (fs_poll_deliver_to(&tty->pollee, POLLIN, poller, false)) //
            return 1;
    tty->reqlen = -1; // inf
    fs_poll_setup(&tty->pollee, poller);
    if (FC_IFLAG(tty, ICANON)) {
        // setup timer
    }
    return 0;
}

int tty_poll_teardown(devst_t *this, struct fs_poller *poller)
{
    tty_t *tty = this->pdata;
    fs_poll_teardown(&tty->pollee, poller);
    return 0;
}

tty_t *tty_init_struct(tty_t *tty, void *data, tty_ioctl_t ctl, tty_iosop_t in,
                       tty_iosop_t out)
{
    if (!tty && !(tty = malloc(sizeof(tty_t)))) return NULL;

    tty->stop = 0;
    tty->pgrp = 0;
    tty->ses_id = 0;
    tty->ses_holders = 0;
    tty->iwaiter = NULL;
    tty->owaiter = NULL;
    tty_buf_init(&tty->ibuf);
    tty_buf_init(&tty->obuf);
    tty->tio = (struct termios){
        .c_iflag = ICRNL | IXON,
        .c_oflag = OPOST | ONLCR,
        .c_cflag = CS8 | CREAD | CLOCAL,
        .c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK,
        .c_cc =
            {
                [VINTR] = 0x03,  /* Ctrl+C */
                [VQUIT] = 0x1C,  /* Ctrl+\ */
                [VERASE] = 0x7F, /* DEL    */
                [VKILL] = 0x15,  /* Ctrl+U */
                [VEOF] = 0x04,   /* Ctrl+D */
                [VTIME] = 0,
                [VMIN] = 1,
                [VSTART] = 0x11, /* Ctrl+Q */
                [VSTOP] = 0x13,  /* Ctrl+S */
                [VSUSP] = 0x1A,  /* Ctrl+Z */
            },
    };
    fs_pollee_init(&tty->pollee);
    tty->data = data;
    tty->ctl = ctl;
    tty->in = in;
    tty->out = out;
    return tty;
}

devst_t *tty_init_device(const char *name, void *tty)
{
    devst_t *dev = dev_new();
    dev->name = strdup(name);
    dev->type = DEV_CHAR;
    dev->subtype = DEV_TTY;
    dev->_init_pctx = tty__init_pctx;
    dev->read = tty_read;
    dev->write = tty_write;
    dev->ioctl = tty_ioctl;
    dev->poll_setup = tty_poll_setup;
    dev->poll_teardown = tty_poll_teardown;
    dev->pdata = tty;
    dev_register(NULL, dev);
    return dev;
}

tty_t *tty_register(tty_t *tty, char *name, void *data, tty_ioctl_t ctl,
                    tty_iosop_t in, tty_iosop_t out)
{
    tty = tty_init_struct(tty, data, ctl, in, out);
    if (!tty) return NULL;
    if (!tty_init_device(name, tty)) return NULL;
    return tty;
}

extern int console_ioctl(void *io, int req, void *argp);
extern ssize_t console_write(void *io, char *s, size_t len);

void __printkcon(char *buf)
{
    if (!fgtty) return;
    size_t i = strlen(buf);
    devst_t d = {.pdata = fgtty};
    tty_write(&d, buf, i);
}

tty_t *tty_init_struct_on_console(int idx)
{
    tty_t *tty = tty_init_struct(NULL, console_get(idx), console_ioctl, NULL,
                                 console_write);
    if (!tty) return NULL;
    ttys[idx - 1] = tty;
    console_init(tty->data);
    return tty;
}

void tty_init()
{
    for (int i = MIN_NR_CONSOLES; i <= MAX_NR_CONSOLES; i++) {
        char name[16];
        devst_t *dev;
        sprintf(name, "tty%d", i);
        dev = tty_init_device(name, (void *)(addr_t)i);
        if (!dev) DEBUGK(K_ERROR, "%s cannot be initialized\n", name);
        if (i == DEF_FG_CONSOLE)
            dev->pdata = fgtty = tty_init_struct_on_console(1);
    }
    if (!fgtty) PANIC("fgtty cannot be initialized\n");
}
