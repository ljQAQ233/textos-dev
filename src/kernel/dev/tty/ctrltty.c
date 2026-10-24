// the special controlling terminal - /dev/tty

#include <textos/dev.h>
#include <textos/dev/tty/tty.h>
#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/task.h>

void ctrltty_detach(task_t *tsk)
{
    if (!tsk->ctrltty) return;

    struct tty *ctty = tsk->ctrltty->pdata;
    assert(ctty->ses_id == tsk->sid);
    assert(ctty->ses_holders > 0);

    if (!--ctty->ses_holders) ctty->ses_id = 0;
    if (tsk->pid == tsk->sid) {
        kill(-ctty->pgrp, SIGHUP);
        kill(-ctty->pgrp, SIGCONT);
        task_t *p = NULL;
        while ((p = task_foreach_next(p))) {
            if (p->sid != tsk->pid) continue;
            if (p == tsk) continue;
            p->ctrltty = NULL;
        }
    }

    tsk->ctrltty = NULL;
}

struct devst *ctrltty_refer(struct task *tsk)
{
    if (tsk->ctrltty) {
        struct tty *ctty = tsk->ctrltty->pdata;
        assert(ctty->ses_id == tsk->sid);
        assert(ctty->ses_holders > 0);
        ctty->ses_holders += 1;
    }
    return tsk->ctrltty;
}

//
// /dev/tty operations
//

static int ctrltty_write(devst_t *dev, void *buf, size_t cnt, ...)
{
    task_t *tsk = task_current();
    if (!tsk->ctrltty) return -ENXIO;
    return tsk->ctrltty->write(tsk->ctrltty, buf, cnt);
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
