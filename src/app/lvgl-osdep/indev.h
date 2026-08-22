typedef struct mouse_context
{
    int evfd;
    int divisor;
    lv_point_t current;
    lv_point_t display_res;
} lv_textos_mouse_context_t;

lv_indev_t * lv_textos_mouse_indev_create(lv_point_t * display_res);

typedef struct kbd_context
{
    int evfd;
} lv_textos_kbd_context_t;
