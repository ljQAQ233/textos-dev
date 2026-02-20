#ifndef __IRQ_H__
#define __IRQ_H__

#define INT_NM 0x07
#define INT_PF 0x0e
#define INT_MF 0x10

#define INT_APIC_ERR 129 // error handler
#define INT_APIC_SPS 130 // spurious handler

#define IRQ_TIMER 0
#define INT_TIMER 0x20

#define IRQ_KEYBOARD 1
#define INT_KEYBOARD 0x21

#define IRQ_PRIDISK 14
#define INT_PRIDISK 0x22

#define IRQ_SECDISK 15
#define INT_SECDISK 0x23

#define INT_E1000 0x24
#define INT_SERIAL 0x25

#define INT_SYSCALL  0x80
#define INT_MSYSCALL 0x81

bool intr_get ();

#define intr_sti() __asm__ volatile ("sti");
#define intr_cli() __asm__ volatile ("cli");

#define UNINTR_AREA_START()                  \
        {                                    \
            bool __intr_stat__ = intr_get(); \
            intr_cli();                      \

#define UNINTR_AREA_END()                    \
        if (__intr_stat__)                   \
            intr_sti();                      \
        }                                    \

#define UNINTR_AREA(opts)                            \
        do {                                         \
            UNINTR_AREA_START();                     \
            opts;                                    \
            UNINTR_AREA_END();                       \
        } while (false);                             \

/* 接下来是 APIC 的舞台! */
void lapic_sendeoi();
void lapic_smp_init_all();
void lapic_smp_sipi_all(u8 vv);

#define _IOAPIC_RTE(vector) ((u64)((u8)vector))

void ioapic_rteset (u8 irq, u64 rte);

#endif
