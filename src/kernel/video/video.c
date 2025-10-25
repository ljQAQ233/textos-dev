#include <textos/video.h>

static u32 hor, ver;
static u32 *fb;
static u64 fb_siz;
static addr_t fb_pa;

#include <textos/assert.h>

void pixel_put(u32 x, u32 y, u32 color)
{
    // BUG: this shouldn't happend, panic here!!!
    if (x >= hor || y >= ver)
        return;

    ASSERTK(x + hor * y < fb_siz);
    u32 *pixel = fb + x + hor * y;
    *pixel = color;
}

u32 pixel_get(u32 x, u32 y)
{
    if (x >= hor || y >= ver)
        return 0;

    ASSERTK(x + hor * y < fb_siz);
    u32 *pixel = fb + x + hor * y;
    return *pixel;
}

void block_put(
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
            pixel_put(i, j, color);
}

void block_transform(
        u32 x,u32 y,
        u32 xe,u32 ye,
        gfx_transformer_t handler,
        void *private
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
            pixel_put(i, j, handler(i - x, j - y, pixel_get(i, j), private));
}

void screen_clear()
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

void screen_info5(void **i_buf, addr_t *i_pa, size_t *i_sz, u32 *i_hor, u32 *i_ver)
{
    if (i_buf) *i_buf = fb;
    if (i_pa) *i_pa = fb_pa;
    if (i_sz) *i_sz = fb_siz;
    if (i_hor) *i_hor = hor;
    if (i_ver) *i_ver = ver;
}

#include <textos/boot.h>

void __video_pre()
{
    if (bmode_get() == BOOT_EFI)
    {
        bconfig_t *b = binfo_get();
        vconfig_t *v = &b->video;
        hor = v->hor;
        ver = v->ver;

        fb = (void *)v->fb;
        fb_siz = v->fb_siz;
    }
}

#include <textos/mm.h>

void __video_tovmm()
{
    size_t pages = DIV_ROUND_UP(fb_siz, PAGE_SIZ);
    vmap_map((u64)fb, __kern_fb_base, pages, PE_RW | PE_P | PTE_G);

    fb_pa = (addr_t)fb;
    fb = (void *)__kern_fb_base;
}

