#include <textos/textos.h>

#include <boot.h>

u64 kernel_init (bconfig_t *config)
{
    return config->magic;
}

