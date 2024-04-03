#ifndef __IRQ_H__
#define __IRQ_H__

#define INT_APIC_ERR 129 // error handler
#define INT_APIC_SPS 130 // spurious handler

#define IRQ_TIMER 0
#define INT_TIMER 0x20

#define IRQ_KEYBOARD 1
#define INT_KEYBOARD 0x21

#define IRQ_MDISK 14
#define INT_MDISK 0x22

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
void lapic_sendeoi ();

#define _IOAPIC_RTE(vector) ((u64)((u8)vector))

void ioapic_rteset (u8 irq, u64 rte);

#endif
