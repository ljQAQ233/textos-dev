// the special controlling terminal - /dev/tty

#include <textos/dev.h>
#include <textos/dev/tty/tty.h>
#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/task.h>

static int ctrltty_write(devst_t *dev, void *buf, size_t cnt, ...)
{
    task_t *tsk = task_current();
    if (!tsk->ctrltty) return -ENXIO;
    return tsk->ctrltty->write(dev, buf, cnt);
}

static int ctrltty_read(devst_t *dev, void *buf, size_t cnt, ...)
{
    task_t *tsk = task_current();
    if (!tsk->ctrltty) return -ENXIO;
    return tsk->ctrltty->read(dev, buf, cnt);
}

static int ctrltty_ioctl(devst_t *dev, int req, void *argp)
{
    task_t *tsk = task_current();
    // TODO: handle TIOCNOTTY
    if (!tsk->ctrltty) return -ENXIO;
    return tsk->ctrltty->ioctl(dev, req, argp);
}
    
static int ctrltty__init_pctx(devst_t *dev, void **pctx)
{
    // just do check
    task_t *tsk = task_current();
    if (!tsk->ctrltty) return -ENXIO;
    return 0;
}

void ctrltty_init()
{
    devst_t *dev = dev_new();
    dev->name = strdup("tty");
    dev->type = DEV_CHAR;
    dev->subtype = DEV_TTY;
    dev->_init_pctx = ctrltty__init_pctx;
    dev->read = ctrltty_read;
    dev->write = ctrltty_write;
    dev->ioctl = ctrltty_ioctl;
    dev->pdata = 0;
    dev_register(NULL, dev);
}
