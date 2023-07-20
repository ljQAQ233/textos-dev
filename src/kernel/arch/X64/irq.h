#ifndef __IRQ_H__
#define __IRQ_H__

#define INT_APIC_ERR 129 // error handler
#define INT_APIC_SPS 130 // spurious handler

#define IRQ_KEYBOARD 1
#define INT_KEYBOARD 0x21

bool intr_get ();

#define intr_sti() __asm__ volatile ("sti");
#define intr_cli() __asm__ volatile ("cli");

/* 接下来是 APIC 的舞台! */
void lapic_sendeoi ();

#define _IOAPIC_RTE(vector) ((u64)((u8)vector))

void ioapic_rteset (u8 irq, u64 rte);

#endif
