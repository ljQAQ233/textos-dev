#include <io.h>
#include <cpu.h>
#include <irq.h>
#include <intr.h>
#include <textos/printk.h>
#include <textos/dev/8259.h>

#include <textos/debug.h>
#include <textos/panic.h>
#include <textos/mm.h>

#define MSR_ID     0x802 // lapic id register
#define MSR_VER    0x803 // lapic version register
#define MSR_TPR    0x808 // task priority register
#define MSR_PPR    0x80a // processor priority register
#define MSR_EOI    0x80b // end of interrupt
#define MSR_LDR    0x80d // logical destination register
#define MSR_SVR    0x80f // spurious interrupt vector register
#define MSR_ISR    0x810 // in-service register   [810H , 817H]
#define MSR_TMR    0x818 // trigger mode register [818H , 81FH]
#define MSR_IRR    0x820 // interrupt request register [820H , 827H]
#define MSR_ESR    0x828 // error status register
#define MSR_CMIC   0x82f // LVT CMIC register
#define MSR_ICR    0x830 // interrupt command register
#define MSR_TM     0x832 // LVT timer register
#define MSR_TRML   0x833 // TheRMaL sensor register
#define MSR_PMR    0x834 // LVT performance monitoring register
#define MSR_LINT0  0x835 // LVT lint0 register
#define MSR_LINT1  0x836 // LVT lint1 register
#define MSR_LERR   0x837 // LVT error register
#define MSR_TICR   0x838 // initial counter for timer
#define MSR_TCCR   0x839 // current counter for timer
#define MSR_TDCR   0x83e // divide configuration for timer
#define MSR_SIPI   0x83f // self IPI

#define LVT_MASK (1 << 16)

void *lapic = NULL;
void *ioapic = NULL;

#define S_TPR(tc, tsc)         ((u32)tc << 4 | (u32)tsc)
#define S_SVR(stat, vector)    ((((u32)stat & 1) << 8) | ((u32)vector & 0xff))

#define S_TM(mode, vector) ((((u32)mode & 0b11) << 17 | (u8)vector))

#define TM_ONESHOT  0b00 // One-Shot Mode
#define TM_PERIODIC 0b01 // Periodic Mode 周期模式
#define TM_TSCDLN   0b10 // TSC-Deadline Mode

void __apic_tovmm ()
{
    // lapic mapping is unused
    vmap_map ((u64)lapic, __lapic_va, 1, PE_RW | PE_P | PE_PCD | PE_PWT, MAP_4K);
    vmap_map ((u64)ioapic, __ioapic_va, 1, PE_RW | PE_P | PE_PCD | PE_PWT, MAP_4K);

    lapic = (void *)__lapic_va;
    ioapic = (void *)__ioapic_va;

}
u64 volatile slice = 0;

__INTR_HANDLER(timer_handler)
{
    lapic_sendeoi();

    slice++;

    if (slice % 100 == 0)
        printk ("One second!!!\n");
}

void lapic_errhandler () { PANIC ("apic handler is not supported!!!"); }
void lapic_spshandler () { ; }

void lapic_set (int x, u32 v)
{
    write_msr(x, v);
}

u32 lapic_get (int x)
{
    return read_msr(x);
}

int x2apic_detect()
{
    u32 eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    return (ecx & (1 << 21));
}

void x2apic_enable()
{
    u64 msr = read_msr(IA32_APIC_BASE);
    write_msr(IA32_APIC_BASE, msr | (1 << 10));
}

#include <textos/dev/pit.h>

/* Apic 不可以启动再禁用后再启动, 除非重启机器 */
void apic_init ()
{
    if (lapic == NULL || ioapic == NULL)
        PANIC ("invalid lapic or ioapic. not detected\n");

    pic_disable(); // 某种意义上,这是废话,大家不要理它...
    
    // write_msr (IA32_APIC_BASE, ((u64)lapic << 12) | (1 << 11));

    if (!x2apic_detect())
        PANIC ("x2apic is not supported!\n");
    else
        x2apic_enable();

    intr_register(INT_APIC_ERR, (ihandler_t)lapic_errhandler);
    intr_register(INT_APIC_SPS, (ihandler_t)lapic_spshandler);

    lapic_set(MSR_LERR, INT_APIC_ERR);

    lapic_set(MSR_ESR, 0);
    lapic_set(MSR_ESR, 0);

    lapic_set(MSR_TPR, S_TPR(0, 0));
    lapic_set(MSR_SVR, S_SVR(true, INT_APIC_SPS));     // 软启用 Apic / APIC Software Enable
    
    lapic_set(MSR_TM, S_TM(TM_ONESHOT, 0) | LVT_MASK); // 屏蔽 Apic Timer 的本地中断
    lapic_set(MSR_TDCR, 0b0000);                       // 设置 除数 (因子) 为 2
    lapic_set(MSR_TICR, 0xFFFFFFFF);                   // 设置 初始计数到最大 (-1)

    pit_sleepms (10);

    lapic_set(MSR_TICR, 0xFFFFFFFF - lapic_get(MSR_TCCR)); // 计算 10ms 的 ticks
    lapic_set(MSR_TM, S_TM(TM_PERIODIC, INT_TIMER));       // 步入正轨, 每 10ms 产生一次时钟中断
    intr_register (INT_TIMER, timer_handler);              // 注册中断函数
    DEBUGK(K_INIT, "apic initialized\n");
}

void lapic_sendeoi()
{
    lapic_set(MSR_EOI, 0);
}

#define IOREGSEL    (ioapic + 0x00)
#define IOWIN       (ioapic + 0x10)
#define IOAPICID    0x00
#define IOAPICVER   0x01
#define IOAPICARB   0x02
#define IOREDTBL    0x10

void ioapic_write(int x, u32 v)
{
    *(volatile u32 *)IOREGSEL = x;
    *(volatile u32 *)IOWIN = v;
}
 
u32 ioapic_read(int x)
{
    *(volatile u32 *)IOREGSEL = x;
    return *(volatile u32 *)IOWIN;
}

extern u8 __gsiget (u8 src);

void ioapic_rteset(u8 irq, u64 rte)
{
    irq = __gsiget(irq);

    int reg = irq * 2 + IOREDTBL;
    ioapic_write(reg, rte & 0xffffffff);
    ioapic_write(reg + 1, rte >> 32);
    DEBUGK(K_PIC, "ioapic set : [#irq] = %016llx\n", irq, rte);
}
 
