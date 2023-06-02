#include <textos/textos.h>

#include <boot.h>

extern void kernel_main();

extern void __video_pre (vconfig_t *v);

void kernel_init (bconfig_t *config)
{
    __video_pre (&config->video);

    kernel_main();
}

