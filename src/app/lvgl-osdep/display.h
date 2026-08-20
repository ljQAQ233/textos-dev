#pragma once

struct disp_context
{
    void *fb;
    size_t fbsz;
};

#define W 1360
#define H 768

lv_display_t *lv_textos_display_create(void);
