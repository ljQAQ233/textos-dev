#include <textos/video.h>

static u32 hor, ver;
static u32 *fb;
static u64 fb_siz;

#include <textos/assert.h>

void pixel_put (
        u32 x,u32 y,
        u32 color
        )
{
    if (x >= hor || y >= ver) {
        return;
    }

    ASSERTK(x + hor * y < fb_siz);
    u32 *pixel = fb + x + hor * y;
    *pixel = color;
}

void block_put (
        u32 x,u32 y,
        u32 xe,u32 ye,
        u32 color
        )
{
    if (x > xe) {
        u32 tmp = x;
        x = xe;
        xe = tmp;
    }

    if (y > ye) {
        u32 tmp = y;
        y = ye;
        ye = tmp;
    }

    for (u32 i = x ; i < xe && i < hor ; i++)
        for (u32 j = y ; j < ye && j < ver ; j++)
            pixel_put (i, j, color);
}

void screen_clear ()
{
    u32 i = fb_siz / 4;
    u32 *p = fb;

    while (i-- && p)
        *p++ = 0;
}

// end 以上 (不包括 end) 的行上拉
void screen_pullup(u32 end, u32 cnt, u32 bg)
{
    if (cnt == 0 || end > ver)
        return;

    u32 *d = fb;
    u32 *s = fb + cnt * hor;
    for (u32 i = 0 ; i < (end - cnt) * hor ; i++)
        *d++ = *s++;
    for (u32 i = 0 ; i < cnt * hor ; i++)
        *d++ = bg;
}

// start 行以下 (包括了 start) 的下拉
void screen_pulldown(u32 start, u32 cnt, u32 bg)
{
    if (cnt == 0)
        return;
    u32 *d = fb + ver * hor - 1;
    u32 *s = d - cnt * hor;
    for (u32 i = 0 ; i < hor * (ver - start - cnt) ; i++)
        *d-- = *s--;
    for (u32 i = 0 ; i < hor * cnt ; i++)
        *d-- = bg;
}

void screen_info (u32 *i_hor, u32 *i_ver)
{
    *i_hor = hor;
    *i_ver = ver;
}

#include <boot.h>

void __video_pre (vconfig_t *v)
{
    hor = v->hor;
    ver = v->ver;

    fb = (void *)v->fb;
    fb_siz = v->fb_siz;
}

#include <textos/mm.h>

void __video_tovmm ()
{
    size_t pages = DIV_ROUND_UP(fb_siz, PAGE_SIZ);
    vmap_map ((u64)fb, __kern_fb_base, pages, PE_RW | PE_P | PTE_G, MAP_4K);

    fb = (void *)__kern_fb_base;
}

