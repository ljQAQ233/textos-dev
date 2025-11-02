#include <textos/boot.h>
#include <textos/klib/string.h>

extern void kernel_main();

extern void __kstack_init();
extern void __idmap_init();

extern void __video_pre();
extern void __mm_pre();
extern void __acpi_pre();

static void *binfo;
static bmode_t bmode;

void kernel_init(long a, long b, long c, long d)
{
    dprintk_set(K_ALL & ~K_SYNC);
    DEBUGK(K_INIT, "kernel_init(%#lx, %#lx, %#lx, %#lx)\n", a, b, c, d);
    if ((u32)a == MULTIBOOT_HEADER_MAGIC)
    {
        binfo = (void *)(long)(u32)b;
        bmode = BOOT_MB1;
    }
    else
    {
        static bconfig_t bconfig;
        bconfig_t *config = (bconfig_t *)a;
        if (config->magic != TEXTOS_BOOT_MAGIC)
            goto die;
        memcpy(&bconfig, config, sizeof(bconfig_t));
        binfo = &bconfig;
        bmode = BOOT_EFI;
    }

    __kstack_init();
    __video_pre();
    __mm_pre();
    __acpi_pre();
    kernel_main();
die:
    return ;
}

void *binfo_get()
{
    return binfo;
}

bmode_t bmode_get()
{
    return bmode;
}
