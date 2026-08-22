#include <fcntl.h>
#include <sys/mman.h>

#include <lvgl/lvgl.h>
#include <lvgl/examples/lv_examples.h>

#include "lvgl-osdep/display.c"
#include "lvgl-osdep/indev.c"
#include "lvgl-osdep/misc.c"

int main()
{
    lv_init();

    // NOTE: we may not have the permission to write to the framebuffer device
    // Due to a lack of a unix-flavor root file system, meaning disabled
    // user/group mechanism, bypassing vfs_permission is a shortcut to test
    lv_display_t *disp = lv_textos_display_create();
    lv_disp_set_default(disp);

    lv_indev_t *cursor = lv_textos_mouse_indev_create(NULL);
    lv_obj_t *img_cursor = lv_image_create(lv_scr_act());
    lv_image_set_src(img_cursor, lv_textos_getimg_mouse(NULL));
    lv_indev_set_cursor(cursor, img_cursor);

    lv_indev_t *kbd = lv_textos_kbd_indev_create();
    LV_UNUSED(kbd);

    // lv_example_win_1();
    // lv_example_calendar_1();

    while (1) {
        lv_tick_inc(1);
        lv_task_handler();
    }

    return 0;
}
