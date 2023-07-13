#include <textos/debug.h>

extern void pmm_init ();

void mm_init ()
{
    pmm_init();
}

#include <boot.h>

extern void __pmm_pre (mconfig_t *m);

void __mm_pre (mconfig_t *m)
{
    __pmm_pre (m);
}
