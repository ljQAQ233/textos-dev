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

void kernel_init(long magic, long info)
{
    dprintk_set(K_ALL & ~K_SYNC);
    DEBUGK(K_INIT, "kernel_init(%#lx, %#lx)\n", magic, info);
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC)
    {
        binfo = (void *)info;
        bmode = BOOT_MB1;
    }
    else
    {
        static bconfig_t bconfig;
        bconfig_t *config = (void *)info;
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
