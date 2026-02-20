#include <cpu.h>
#include <irq.h>
#include <textos/ap.h>
#include <textos/mycpu.h>
#include <textos/mm/vmm.h>
#include <textos/printk.h>
#include <textos/dev/pit.h>
#include <textos/klib/string.h>

extern struct {
    uint jmp;
    uint lma;
    uint size;
    uint ppgt;
    uint idlk;
    char code[0];
} __apboot;

extern char __gdtr[];

void ap_init()
{
    if (cpu_count() == 1)
        return;
    addr_t pa = __apboot.lma;
    size_t sz = __apboot.size;
    vmap_map(pa, pa, DIV_ROUND_UP(sz, PAGE_SIZE), PE_P | PE_RW);
    __apboot.ppgt = get_kppgt();
    __apboot.idlk = 0;
    memcpy((void *)pa, &__apboot, sz);
    printk("trampoline at %p (size %dB)\n", pa, sz);
    u8 vv = pa >> 12;
    lapic_smp_init_all();
    pit_sleepms(10);
    lapic_smp_sipi_all(vv);
    pit_sleepms(1);
    lapic_smp_sipi_all(vv);
}

extern char __gdtr[];

void ap_main()
{
    load_gdt(__gdtr);
    reload_segs(KERN_DATA_SEG << 3, KERN_CODE_SEG << 3);
    printk("this is from ap!!!!!!\n");
    intr_cli();
    halt();
}
