#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

#include <lvgl/lvgl.h>

#include "lvgl-osdep/display.h"

int main()
{
    lv_display_t *disp;
    lv_group_t *group;
    
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
