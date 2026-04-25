#include <io.h>
#include <cpu.h>
#include <irq.h>
#include <intr.h>
#include <textos/dev/8259.h>

#include <textos/panic.h>
#include <textos/mm.h>

/* LApiC's registers */
#define LAPIC_ID   0x20
#define LAPIC_VER  0x30
#define LAPIC_TPR  0x80
#define LAPIC_APR  0x90
#define LAPIC_PPR  0xA0
#define LAPIC_EOI  0xB0
#define LAPIC_RRD  0xC0
#define LAPIC_LDR  0xD0   // Logical dest register
#define LAPIC_DFR  0xE0   // Destination fmt register
#define LAPIC_SVR  0xF0   // Spurious Interrupt Vector Register
#define LAPIC_ISR  0x100
#define LAPIC_TMR  0x180
#define LAPIC_IRR  0x200
#define LAPIC_ESR  0x280  // Error status register
#define LAPIC_CMCI 0x2F0  // LVT Corrected Machine Check Interrupt Register
#define LAPIC_ICR  0x300
#define LAPIC_TM   0x320  // LVT timer register
#define LAPIC_TRML 0x330  // TheRMaL sensor register
#define LAPIC_PC   0x340  // Performance counter
#define LAPIC_INT0 0x350
#define LAPIC_INT1 0x360
#define LAPIC_LERR  0x370 // LVT Error Register
#define LAPIC_TICR 0x380  // Initial Counter Register for timer
#define LAPIC_TCCR 0x390  // Current Counter Register for timer
#define LAPIC_DCR  0x3E0  // Divide Configuration Register for timer

#define LVT_MASK (1 << 16)

void *lapic = NULL;
void *ioapic = NULL;

#define LAPIC_GET(reg)         (*((u32 volatile *)(lapic + reg)))
#define LAPIC_SET(reg, val)    (*((u32 volatile *)(lapic + reg)) = (u32)(val))

#define S_TPR(tc, tsc)         ((u32)tc << 4 | (u32)tsc)
#define S_SVR(stat, vector)    ((((u32)stat & 1) << 8) | ((u32)vector & 0xff))

void __apic_tovmm ()
{
    vmap_map ((u64)lapic, __lapic_va, 1, PE_RW | PE_P, MAP_4K);
    vmap_map ((u64)ioapic, __ioapic_va, 1, PE_RW | PE_P, MAP_4K);

    lapic = (void *)__lapic_va;
    ioapic = (void *)__ioapic_va;
}

void lapic_errhandler () { PANIC ("apic handler is not supported!!!"); }
void lapic_spshandler () { ; }

/* Apic 不可以启动再禁用后再启动, 除非重启机器 */
void apic_init ()
{
    if (lapic == NULL || ioapic == NULL)
        PANIC ("invalid lapic or ioapic. not detected\n");

    pic_disable(); // 某种意义上,这是废话,大家不要理它...

    // write_msr (IA32_APIC_BASE, ((u64)lapic << 12) | (1 << 11));

    intr_register (INT_APIC_ERR, (ihandler_t)lapic_errhandler);
    intr_register (INT_APIC_SPS, (ihandler_t)lapic_spshandler);

    LAPIC_SET(LAPIC_LERR, INT_APIC_ERR);

    LAPIC_SET(LAPIC_ESR, 0);
    LAPIC_SET(LAPIC_ESR, 0);

    LAPIC_SET(LAPIC_TPR, S_TPR(0, 0));
    LAPIC_SET(LAPIC_SVR, S_SVR(true, INT_APIC_SPS));  // 软启用 Apic / APIC Software Enable
}

void lapic_sendeoi ()
{
    LAPIC_SET (LAPIC_EOI, 0);
}

void ioapic_rteset (u8 irq, u64 rte)
{
    u32 *selector = (u32 *)(ioapic);
    u32 *data     = (u32 *)(ioapic + 0x10);

    *selector = irq * 2 + 0x10;
    *data = rte & 0xFFFFFFFF;

    *selector = irq * 2 + 0x10 + 1;
    *data = rte >> 32;
}

