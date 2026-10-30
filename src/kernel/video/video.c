#include <textos/boot.h>
#include <textos/mm.h>
#include <textos/video.h>

static u32 hor, ver;
static u32 *fb;
static u64 fb_siz;
static addr_t fb_pa;

void pixel_put(u32 x, u32 y, u32 color)
{
    assert(x < hor && y < ver);
    u32 *pixel = fb + x + hor * y;
    *pixel = color;
}

u32 pixel_get(u32 x, u32 y)
{
    assert(x < hor && y < ver);
    u32 *pixel = fb + x + hor * y;
    return *pixel;
}

void block_put(u32 x, u32 y, u32 xe, u32 ye, u32 color)
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

    for (u32 i = x; i < xe && i < hor; i++)
        for (u32 j = y; j < ye && j < ver; j++)
            pixel_put(i, j, color);
}

void block_transform(u32 x, u32 y, u32 xe, u32 ye, gfx_transformer_t handler,
                     void *private)
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

    for (u32 i = x; i < xe && i < hor; i++)
        for (u32 j = y; j < ye && j < ver; j++)
            pixel_put(i, j, handler(i - x, j - y, pixel_get(i, j), private));
}

void screen_clear()
{
    u32 i = fb_siz / 4;
    u32 *p = fb;

    while (i-- && p)
        *p++ = 0;
}

void screen_pullup(u32 cnt, u32 bg, u32 vpm_top, u32 vpm_bot)
{
    if (cnt == 0) return;
    if (vpm_top >= hor) return;
    if (vpm_bot >= hor) return;

    u32 *dst = fb + vpm_top * hor;
    u32 *src = dst + cnt * hor;
    for (u32 i = 0; i < (ver - vpm_top - vpm_bot - cnt) * hor; i++)
        *dst++ = *src++;
    for (u32 i = 0; i < cnt * hor; i++)
        *dst++ = bg;
}

void screen_pulldown(u32 cnt, u32 bg, u32 vpm_top, u32 vpm_bot)
{
    if (cnt == 0) return;
    if (vpm_top >= hor) return;
    if (vpm_bot >= hor) return;
    u32 *dst = fb + hor + (ver - vpm_bot) * hor - 1;
    u32 *src = dst - cnt * hor;
    for (u32 i = 0; i < hor * (ver - vpm_top - vpm_bot - cnt); i++)
        *dst-- = *src--;
    for (u32 i = 0; i < hor * cnt; i++)
        *dst-- = bg;
}

bool screen_istxt()
{
    return fb == 0;
}

void screen_info(u32 *i_hor, u32 *i_ver)
{
    *i_hor = hor;
    *i_ver = ver;
}

void screen_info5(void **i_buf, addr_t *i_pa, size_t *i_sz, u32 *i_hor,
                  u32 *i_ver)
{
    if (i_buf) *i_buf = fb;
    if (i_pa) *i_pa = fb_pa;
    if (i_sz) *i_sz = fb_siz;
    if (i_hor) *i_hor = hor;
    if (i_ver) *i_ver = ver;
}

void __video_pre()
{
    if (bmode_get() == BOOT_EFI) {
        bconfig_t *b = binfo_get();
        hor = b->hor;
        ver = b->ver;

        fb = (void *)b->fb;
        fb_siz = b->fb_siz;
    }
}

void __video_tovmm()
{
    if (bmode_get() == BOOT_EFI) {
        size_t pages = DIV_ROUND_UP(fb_siz, PAGE_SIZ);
        vmap_map((u64)fb, __kern_gfx_base, pages, PE_RW | PE_P | PTE_G);

        fb_pa = (addr_t)fb;
        fb = (void *)__kern_gfx_base;
    }
}
