#include <textos/dev.h>
#include <textos/dev/event.h>
#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/task.h>

struct receiver
{
    struct event ev;
    task_t *waiter;
};

void event_deliver(devst_t *this)
{
    struct receiver *rx = this->pdata;
    if (rx->waiter) {
        task_unblock(rx->waiter, 0);
        rx->waiter = 0;
    }
}

void event_push_any(devst_t *this, struct event *ev)
{
    struct receiver *rx = this->pdata;
    memcpy(&rx->ev, ev, sizeof(struct receiver));
    event_deliver(this);
}

void event_push_keyboard(devst_t *this, keysym_t sym)
{
    struct receiver *rx = this->pdata;
    rx->ev.type = EV_KEYBOARD;
    rx->ev.sym = sym;
    event_deliver(this);
}

void event_push_mouse(devst_t *this, keysym_t status, int dx, int dy)
{
    struct receiver *rx = this->pdata;
    rx->ev.type = EV_MOUSE;
    rx->ev.sym = status;
    rx->ev.m.dx = dx;
    rx->ev.m.dy = dy;
    event_deliver(this);
}

//
// driver interface code
//

static int event_read_one(struct receiver *rx, struct event *ev)
{
    if (rx->ev.type == EV_NONE) {
        ev->type = EV_NONE;
        rx->waiter = task_current();
        int ret = task_block(NULL, NULL, TASK_BLK, 0);
        if (ret < 0) return ret;
    }
    memcpy(ev, &rx->ev, sizeof(*ev));
    rx->ev.type = EV_NONE;
    return 0;
}

static int event_read(devst_t *dev, void *buf, size_t cnt)
{
    struct receiver *rx = dev->pdata;
    size_t i = 0;
    while (i < cnt) {
        event_read_one(rx, buf + i);
        i += sizeof(struct event);
    }
    return i;
}

static int event_ioctl(devst_t *dev, int req, void *argp)
{
    return -EINVAL;
}

devst_t *event_register(enum event_type type)
{
    struct receiver *data = malloc(sizeof(struct receiver));
    data->waiter = 0;
    data->ev.type = EV_NONE;

    devst_t *evdev = dev_new();
    evdev->name = "event";
    evdev->type = DEV_CHAR;
    evdev->subtype = DEV_EVENT;
    evdev->read = event_read;
    evdev->write = noopt_perm,
    evdev->ioctl = event_ioctl,
    evdev->pdata = data;
    dev_register(NULL, evdev);
    return evdev;
}
