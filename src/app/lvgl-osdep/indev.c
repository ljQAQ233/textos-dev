#include <fcntl.h>
#include <sys/event.h>
#include <sys/keys.h>
#include <unistd.h>

#include <lvgl/lvgl.h>

#include "indev.h"

static void _mouse_indev_event_cb(lv_event_t *e)
{
    lv_indev_t *indev;
    lv_textos_mouse_context_t *indev_ctx;

    if (lv_event_get_code(e) != LV_EVENT_DELETE) return;

    indev = (lv_indev_t *)lv_event_get_user_data(e);
    if (indev == NULL) return;

    indev_ctx = (lv_textos_mouse_context_t *)lv_indev_get_user_data(indev);
    lv_indev_set_user_data(indev, NULL);

    if (indev_ctx != NULL) lv_free(indev_ctx);
}

static void _mouse_indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    ssize_t ret;
    struct event ev;
    lv_textos_mouse_context_t *indev_ctx;

    indev_ctx = (lv_textos_mouse_context_t *)lv_indev_get_user_data(indev);
    LV_ASSERT_NULL(indev_ctx);

    ret = read(indev_ctx->evfd, &ev, sizeof(ev));
    if (ret <= 0) return;
    if (ev.type == EV_NONE) return;

    int dx = ev.m.dx / indev_ctx->divisor;
    int dy = ev.m.dy / indev_ctx->divisor;

    indev_ctx->current.x += dx;
    indev_ctx->current.y += dy;
    if (indev_ctx->current.x < 0) indev_ctx->current.x = 0;
    if (indev_ctx->current.x >= indev_ctx->display_res.x)
        indev_ctx->current.x = indev_ctx->display_res.x - 1;
    if (indev_ctx->current.y < 0) indev_ctx->current.y = 0;
    if (indev_ctx->current.y >= indev_ctx->display_res.y)
        indev_ctx->current.y = indev_ctx->display_res.y - 1;
    data->point.x = indev_ctx->current.x;
    data->point.y = indev_ctx->current.y;
    data->state |=
        (ev.sym & (1 << 16)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

lv_indev_t *lv_textos_mouse_indev_create(lv_point_t *display_res)
{
    int evfd;
    lv_indev_t *indev = NULL;
    lv_textos_mouse_context_t *indev_ctx = NULL;

    evfd = open("/dev/event/mouse0", O_RDONLY | O_NONBLOCK);

    indev_ctx = lv_calloc(1, sizeof(lv_textos_mouse_context_t));
    LV_ASSERT_MALLOC(indev_ctx);

    indev_ctx->evfd = evfd;
    indev_ctx->current.x = 0;
    indev_ctx->current.y = 0;
    indev_ctx->divisor = 2 / 2;
    if (display_res != NULL) {
        indev_ctx->display_res.x = display_res->x;
        indev_ctx->display_res.y = display_res->y;
    } else {
        indev_ctx->display_res.x =
            lv_display_get_horizontal_resolution(lv_display_get_default());
        indev_ctx->display_res.y =
            lv_display_get_vertical_resolution(lv_display_get_default());
    }

    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_user_data(indev, indev_ctx);
    lv_indev_add_event_cb(indev, _mouse_indev_event_cb, LV_EVENT_DELETE, indev);
    lv_indev_set_read_cb(indev, _mouse_indev_read_cb);

    return indev;
}

static void _kbd_indev_event_cb(lv_event_t *e)
{
    lv_indev_t *indev;
    lv_textos_kbd_context_t *indev_ctx;

    if (lv_event_get_code(e) != LV_EVENT_DELETE) return;

    indev = (lv_indev_t *)lv_event_get_user_data(e);
    if (indev == NULL) return;

    indev_ctx = (lv_textos_kbd_context_t *)lv_indev_get_user_data(indev);
    lv_indev_set_user_data(indev, NULL);

    if (indev_ctx != NULL) lv_free(indev_ctx);
}

static uint32_t _kbd_from_event(struct event *e)
{
    int value = e->sym & ~KEY_S_MASK;
    int status = e->sym & KEY_S_MASK;
    LV_UNUSED(status);

    switch (value) {
    case KEY_UP:
        return LV_KEY_UP;
    case KEY_DOWN:
        return LV_KEY_DOWN;
    case KEY_LEFT:
        return LV_KEY_LEFT;
    case KEY_RIGHT:
        return LV_KEY_RIGHT;
    case KEY_DELETE:
        return LV_KEY_DEL;
    case KEY_HOME:
        return LV_KEY_HOME;
    case KEY_END:
        return LV_KEY_END;
    case KEY_ESC:
        return LV_KEY_ESC;
    default:
        break;
    }
    return KEYCHR(value);
}

static void _kbd_indev_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    ssize_t ret;
    struct event ev;
    lv_textos_kbd_context_t *indev_ctx;

    indev_ctx = (lv_textos_kbd_context_t *)lv_indev_get_user_data(indev);
    LV_ASSERT_NULL(indev_ctx);

    ret = read(indev_ctx->evfd, &ev, sizeof(ev));
    if (ret <= 0) return;
    if (ev.type == EV_NONE) return;
    data->key = _kbd_from_event(&ev);
}

lv_indev_t *lv_textos_kbd_indev_create()
{
    int evfd;
    lv_indev_t *indev = NULL;
    lv_textos_kbd_context_t *indev_ctx = NULL;

    evfd = open("/dev/event/keyboard0", O_RDONLY | O_NONBLOCK);

    indev_ctx = lv_calloc(1, sizeof(lv_textos_kbd_context_t));
    LV_ASSERT_MALLOC(indev_ctx);

    indev_ctx->evfd = evfd;

    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_user_data(indev, indev_ctx);
    lv_indev_add_event_cb(indev, _kbd_indev_event_cb, LV_EVENT_DELETE, indev);
    lv_indev_set_read_cb(indev, _kbd_indev_read_cb);

    return indev;
}
