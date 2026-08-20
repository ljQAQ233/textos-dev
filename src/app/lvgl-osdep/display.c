#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

#include <lvgl/lvgl.h>

#include "display.h"

static void *fb;

static void _disp_event_cb(lv_event_t *e)
{
    lv_display_t *disp;

    disp = (lv_display_t *)lv_event_get_user_data(e);
}

static void _disp_flush_cb(lv_display_t *display, const lv_area_t *area,
                           uint8_t *px_map)
{
    lv_display_flush_ready(display);
}

lv_display_t *lv_textos_display_create(void)
{
    int fd;
    lv_display_t *disp;
    struct disp_context *ctx;

    fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        perror("open");
        return NULL;
    }

    fb = mmap(NULL, W * H * 4, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    disp = lv_display_create(W, H); // TODO
    lv_display_add_event_cb(disp, _disp_event_cb, LV_EVENT_ALL, NULL);
    lv_display_set_flush_cb(disp, _disp_flush_cb);

    ctx = lv_malloc(sizeof(struct disp_context));
    ctx->fb = fb;
    ctx->fbsz = W * H * 4;
    lv_display_set_user_data(disp, ctx);
    lv_display_set_buffers(disp, ctx->fb, NULL, ctx->fbsz,
                           LV_DISPLAY_RENDER_MODE_DIRECT);

    return disp;
}
