extern void pmm_init();
extern void vmap_init();
extern void vmap_initvm();
extern void heap_init();
extern void pvpage_init();
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

extern void __pmm_pre();

void __mm_pre()
{
    __pmm_pre();
}
