#include <textos/ap.h>
#include <textos/mycpu.h>
#include <textos/mm/vmm.h>
#include <textos/mm/heap.h>
#include <textos/klib/string.h>

addr_t *__mycpu_ptrs;
extern void arch_mycpu_init();
extern char __mycpu_start[], __mycpu_end[];

void mycpu_init()
{
    int ncpu = cpu_count();
    __mycpu_ptrs = malloc(sizeof(addr_t) * ncpu);
    for (int i = 0 ; i < ncpu ; i++)
    {
        int mysz = __mycpu_end - __mycpu_start;
        char *base = malloc(mysz);
        memcpy(base, __mycpu_start, mysz);
        get_cpu_base(i) = (addr_t)(base - __mycpu_start);
        get_cpu_var(i, cpu_id) = i;
        get_cpu_var(i, cpu_kstk) = (addr_t)vmm_allocpages(8, PE_P | PE_RW);
        DEBUGK(K_LOGK, "mycpu[#%d] base at %p\n", i, base);
    }
    arch_mycpu_init();
}

// common attributes
MYCPU_DEFINE(int, cpu_id);
MYCPU_DEFINE(addr_t, cpu_kstk);
