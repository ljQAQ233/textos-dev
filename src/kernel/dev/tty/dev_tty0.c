// impl for /dev/tty0
// tty0 always points to foreground tty. note that writing after
// switching fgtty doesn't lead to writing to new fgtty
#include <textos/cmdline.h>
#include <textos/dev.h>
#include <textos/dev/internal.h>
#include <textos/dev/tty/tty.h>
#include <textos/errno.h>

__CMDLINE_USED("console");
static dev_opts_t *op;
extern struct tty *__fgtty;

static int tty0__init_pctx(devst_t *dev, void **pctx)
{
    if (!__fgtty) return -ENODEV;
    *pctx = __fgtty;
    return 0;
}

static int tty0_write(devst_t *dev, void *buf, size_t cnt, ...)
{
    struct fs_openctx *openctx = dev_get_openctx(cnt);
    struct tty *tty = (struct tty *)openctx->pctx;
    devst_t d = {.pdata = tty};
    return op->write(&d, buf, cnt);
}

static int tty0_read(devst_t *dev, void *buf, size_t cnt, ...)
{
    struct fs_openctx *openctx = dev_get_openctx(cnt);
    struct tty *tty = (struct tty *)openctx->pctx;
    devst_t d = {.pdata = tty};
    return op->read(&d, buf, cnt);
}

static int tty0_ioctl(devst_t *dev, int req, void *argp, ...)
{
    struct fs_openctx *openctx = dev_get_openctx(argp);
    struct tty *tty = (struct tty *)openctx->pctx;
    devst_t d = {.pdata = tty};
    return op->ioctl(&d, req, argp);
}

static dev_opts_t tty0_op = {
    ._init_pctx = tty0__init_pctx,
    .write = tty0_write,
    .read = tty0_read,
    .ioctl = tty0_ioctl,
    .driver_name = "tty-fgtty (tty0)",
};

void tty0_init()
{
    tty_getops(&op);
    tty0_op.poll_setup = op->poll_setup;
    tty0_op.poll_teardown = op->poll_teardown;

    devst_t *dev = dev_new();
    dev->name = "tty0";
    dev->type = DEV_CHAR;
    dev->subtype = DEV_TTY;
    dev->op = &tty0_op;
    dev->pdata = NULL;
    dev_register(NULL, dev);
}
