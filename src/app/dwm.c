#include <app/api.h>
#include <lvgl/lvgl.h>
#include <stdio.h>

#pragma lib("lvgl/lvgl.o")

typedef struct
{
    void *buf;
    size_t bufsz;
} disp_ctx_t;

static void _disp_event_cb(lv_event_t * e)
{
    lv_display_t * disp;

    disp = (lv_display_t *)lv_event_get_user_data(e);
}

void *buf;

static void _disp_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    lv_display_flush_ready(display);
}

#define W 1360
#define H 768

lv_display_t *lv_textos_display_create(void)
{
    lv_display_t *disp;
    disp_ctx_t *ctx;

    disp = lv_display_create(W, H); // TODO
    lv_display_add_event_cb(disp, _disp_event_cb, LV_EVENT_ALL, NULL);
    lv_display_set_flush_cb(disp, _disp_flush_cb);

    ctx = lv_malloc(sizeof(disp_ctx_t));
    ctx->buf = buf;
    ctx->bufsz = W * H * 4;
    lv_display_set_user_data(disp, ctx);
    lv_display_set_buffers(disp, ctx->buf, NULL, ctx->bufsz, LV_DISPLAY_RENDER_MODE_DIRECT);

    return disp;
}

int main()
{
    lv_display_t *disp;
    lv_group_t *group;
    int fd = open("/dev/fb0", O_RDWR);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    
    buf = mmap(NULL, W * H * 4, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    
    lv_init();

    disp = lv_textos_display_create();
    lv_disp_set_default(disp);

    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Hello, LVGL!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    while (1)
    {
        lv_tick_inc(1);
        lv_task_handler();
    }

    return 0;
}
