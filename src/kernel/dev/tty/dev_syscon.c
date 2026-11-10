// impl for /dev/console
// we only support one console which only associated with single tty
#include <textos/cmdline.h>
#include <textos/dev.h>
#include <textos/dev/tty/tty.h>
#include <textos/errno.h>
#include <textos/klib/arraylen.h>

__CMDLINE_USED("console");
static dev_opts_t *op;
static struct tty *out;

static int syscon__init_pctx(devst_t *dev, void **pctx)
{
    if (!out) return -ENODEV;
    return 0;
}

static int syscon_write(devst_t *dev, void *buf, size_t cnt, ...)
{
    devst_t d = {.pdata = out};
    return op->write(&d, buf, cnt);
}

static int syscon_read(devst_t *dev, void *buf, size_t cnt, ...)
{
    devst_t d = {.pdata = out};
    return op->read(&d, buf, cnt);
}

static int syscon_ioctl(devst_t *dev, int req, void *argp, ...)
{
    devst_t d = {.pdata = out};
    return op->ioctl(&d, req, argp);
}

static dev_opts_t syscon_op = {
    ._init_pctx = syscon__init_pctx,
    .write = syscon_write,
    .read = syscon_read,
    .ioctl = syscon_ioctl,
    .driver_name = "tty-syscon",
};

void syscon_init()
{
    devst_t *out_dev = NULL;
    char *out_name;
    char *custom_con = cmdline_get_str_or("console", NULL);
    char *candicates[] = {
        custom_con,
        "tty1",
        "ttyS0",
    };

    for (int i = 0; i < arraylen(candicates) && !out_dev; i++)
        if ((out_name = candicates[i]) != NULL)
            out_dev = dev_lookup_name(out_name);

    if (!out_dev) {
        out_dev = dev_lookup_type(DEV_TTY, 0);
        if (out_dev) out_name = out_dev->name;
    }
    if (out_dev) out = out_dev->pdata;
    DEBUGK(K_INFO, "%s selected as console output device\n", out_name);

    tty_getops(&op);
    syscon_op.poll_setup = op->poll_setup;
    syscon_op.poll_teardown = op->poll_teardown;

    devst_t *dev = dev_new();
    dev->name = "console";
    dev->type = DEV_CHAR;
    dev->subtype = DEV_TTY;
    dev->op = &syscon_op;
    dev->pdata = NULL;
    dev_register(NULL, dev);

    if (custom_con) cmdline_free_str(custom_con);
}
