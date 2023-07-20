#ifndef __IRQ_H__
#define __IRQ_H__

#define INT_APIC_ERR 129 // error handler
#define INT_APIC_SPS 130 // spurious handler

#define IRQ_KEYBOARD 1
#define INT_KEYBOARD 0x21

#define intr_get ()

void intr_sti ();

void intr_cli ();

/* 接下来是 APIC 的舞台! */
void lapic_sendeoi ();

#define _IOAPIC_RTE(vector) ((u64)((u8)vector))

void ioapic_rteset (u8 irq, u64 rte);

#endif
