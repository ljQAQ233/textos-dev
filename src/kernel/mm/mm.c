
extern void pmm_init ();
extern void vmap_init ();
extern void vmap_initvm ();
extern void heap_init ();
extern void pvpage_init ();
extern void pagefault_init();

void mm_init ()
{
    pmm_init();
    vmap_init();

    heap_init();

    vmap_initvm();
    pvpage_init();
    pagefault_init();
}

#include <boot.h>

extern void __pmm_pre (mconfig_t *m);

void __mm_pre (mconfig_t *m)
{
    __pmm_pre (m);
}
