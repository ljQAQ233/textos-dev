#include <cpu.h>
#include <irq.h>
#include <textos/ap.h>
#include <textos/mycpu.h>
#include <textos/mm/vmm.h>
#include <textos/dev/pit.h>
#include <textos/klib/string.h>

char code16[] = {
    0xf4,      // hlt
    0xeb, 0xfe // jmp $
};

void ap_init()
{
    void *va = vmm_allocvrt(1);
    addr_t pa = 0;
    vmap_map(pa, (addr_t)va, 1, PE_P | PE_RW);
    memcpy(va, code16, sizeof(code16));
    u8 vv = pa >> 12;
    if (cpu_count() == 1)
        return;
    lapic_smp_init_all();
    pit_sleepms(10);
    lapic_smp_sipi_all(vv);
    pit_sleepms(1);
    lapic_smp_sipi_all(vv);
}
