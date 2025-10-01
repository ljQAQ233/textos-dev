// run with `./ldbase` you will see different results
#include <stdio.h>
#include <link.h>

int main()
{
    extern struct r_debug _r_debug;
    struct r_debug *r = &_r_debug;
    // extern struct r_debug *_dl_debug_addr;
    // struct r_debug *r = _dl_debug_addr;
    printf("r_ldbase: %p\n", (void*)r->r_ldbase);
    
    ElfW(Addr) offset = r->r_brk - r->r_ldbase;
    printf("offset of _dl_debug_state: %#lx\n", offset);

    printf("_r_debug at %p\n", r);
    Elf64_Dyn *dyn = (void *)&_DYNAMIC;
    for ( ; dyn->d_tag != DT_NULL ; dyn++)
        if (dyn->d_tag == DT_DEBUG)
            printf("DT_DEBUG is %#lx\n", dyn->d_un.d_ptr);
    
    return 0;
}
