#include <textos/dev.h>
#include <textos/dev/event.h>
#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/task.h>

void event_deliver(struct event_registry *this)
{
    struct event_client *ec = this->client;
    if (ec->waiter) {
        task_unblock(ec->waiter, 0);
        ec->waiter = 0;
    }
}

void event_push_any(struct event_registry *this, struct event *ev)
{
    struct event_client *ec = this->client;
    memcpy(&ec->ev, ev, sizeof(struct event_registry));
    event_deliver(this);
}

void event_push_keyboard(struct event_registry *this, keysym_t sym)
{
    struct event_client *ec = this->client;
    ec->ev.type = EV_KEYBOARD;
    ec->ev.sym = sym;
    event_deliver(this);
}

void event_push_mouse(struct event_registry *this, keysym_t status, int dx,
                      int dy)
{
    struct event_client *ec = this->client;
    ec->ev.type = EV_MOUSE;
    ec->ev.sym = status;
    ec->ev.m.dx = dx;
    ec->ev.m.dy = dy;
    event_deliver(this);
}

//
// driver interface code
//

static int event__init_pctx(devst_t *dev, void **pctx)
{
    struct event_client *ec = malloc(sizeof(struct event_client));
    ec->waiter = 0;
    ec->ev.type = EV_NONE;
    *pctx = ec;

    struct event_registry *evreg = dev->pdata;
    evreg->client = ec;
    DEBUGK(K_DEBUG, "event client is set-up for task %d\n",
           task_current()->pid);
    return 0;
}

static int event__fini_pctx(devst_t *dev, void **pctx)
{
    struct event_client *ec = *pctx;
    free(ec);

    struct event_registry *evreg = dev->pdata;
    evreg->client = NULL;
    DEBUGK(K_DEBUG, "event client down for task %d\n", task_current()->pid);
    return 0;
}

static int event_read_one(struct event_registry *evreg, struct event *ev)
{
    struct event_client *ec = evreg->client;
    if (ec->ev.type == EV_NONE) {
        ev->type = EV_NONE;
        ec->waiter = task_current();
        int ret = task_block(NULL, NULL, TASK_BLK, 0);
        if (ret < 0) return ret;
    }
    memcpy(ev, &ec->ev, sizeof(*ev));
    ec->ev.type = EV_NONE;
    return 0;
}

static int event_read(devst_t *dev, void *buf, size_t cnt, ...)
{
    struct event_registry *evreg = dev->pdata;
    size_t i = 0;
    while (i < cnt) {
        event_read_one(evreg, buf + i);
        i += sizeof(struct event);
    }
    return i;
}

static int event_ioctl(devst_t *dev, int req, void *argp)
{
    return -EINVAL;
}

struct event_registry *event_register(enum event_type type)
{
    devst_t *evdev = dev_new();
    struct event_registry *evreg = malloc(sizeof(struct event_registry));
    evdev->name = "event";
    evdev->type = DEV_CHAR;
    evdev->subtype = DEV_EVENT;
    evdev->_init_pctx = event__init_pctx;
    evdev->_fini_pctx = event__fini_pctx;
    evdev->read = event_read;
    evdev->write = noopt_perm;
    evdev->ioctl = event_ioctl;
    evdev->pdata = evreg;
    dev_register(NULL, evdev);

    evreg->client = NULL;
    evreg->evdev = evdev;
    return evreg;
}
