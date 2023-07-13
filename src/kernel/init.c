#include <textos/textos.h>
#include <textos/debug.h>

#include <boot.h>

extern void kernel_main();

extern void __video_pre (vconfig_t *v);
extern void __mm_pre (mconfig_t *m);

void kernel_init (bconfig_t *config)
{
    dprintk_set(K_ALL & ~K_SYNC);
    DEBUGK(K_INIT, "kernel_init(%p)\n", config);

    __video_pre (&config->video);
    __mm_pre (&config->memory);

    kernel_main();
}

