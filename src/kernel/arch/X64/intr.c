#include <cpu.h>
#include <gdt.h>
#include <intr.h>
#include <string.h>
#include <textos/assert.h>
#include <textos/printk.h>

typedef struct _packed {
    u16 limit;
    u64 base;
} idtr_t;

typedef struct _packed {
    u16 offset_low  : 16;
    u16 selector    : 16;
    u8  ist         : 2 ;
    u8  rev         : 6 ;
    u8  type        : 4 ;
    u8  zero        : 1 ;
    u8  dpl         : 2 ;
    u8  present     : 1 ;
    u16 offset_mid  : 16;
    u32 offset_high : 32;
    u32 rev2        : 32;
} idt_t;

STATIC_ASSERT (sizeof(idt_t) == 16, "");

#define ENTRY_SIZ   16
#define HANDLER_MAX 256

static idt_t idts[HANDLER_MAX];
static idtr_t idtr;

static void _idt_set_entry (size_t i,u64 offset, u16 selector, u8 type, u8 dpl, u8 present)
{
    idt_t *entry = &idts[i];

    entry->offset_low = offset & 0xffff;
    entry->offset_mid = offset >> 16 & 0xffff;
    entry->offset_high = offset >> 32;
    entry->selector = selector;

    entry->dpl  = dpl;
    entry->type = type;

    entry->ist  = 0;
    entry->present = 1;
}

#define GATE_INT  0b1110
#define GATE_TRAP 0b1111

#define _IDT_SET_ENTRY(i, offset) \
    _idt_set_entry (i, offset, KERN_CODE_SEG << 3, GATE_INT, 0, 1)

//

static const char *exception_msg[] = {
    "#DE Divide Error\0",
    "#DB Debug Exception\0",
    "--- NMI Interrupt\0",
    "#BP Breakpoint\0",
    "#OF Overflow\0",
    "#BR BOUND Range Exceeded\0",
    "#UD Invalid Opcode (Undefined Opcode)\0",
    "#NM Device Not Available (No Math Coprocessor)\0",
    "#DF Double Fault\0",
    "--- Coprocessor Segment Overrun (reserved)\0",
    "#TS Invalid TSS\0",
    "#NP Segment Not Present\0",
    "#SS Stack-Segment Fault\0",
    "#GP General Protection\0",
    "#PF Page Fault\0",
    "---  (Intel reserved. Do not use)\0",
    "#MF x87 FPU Floating-Point Error (Math Fault)\0",
    "#AC Alignment Check\0",
    "#MC Machine Check\0",
    "#XM SIMD Floating-Point Exception\0",
    "#VE Virtualization Exception\0",
    "#CP Control Protection Exception\0",
};

static const char *rev_msg  = "--- Intel reserved. Do not use\0";
static const char *user_msg = "--- User defined (Non-reserved) Interrupts\0";

static inline const char *msgget (u8 Vector)
{
    if (Vector < 22)
        return exception_msg[Vector];
    else if (Vector > 31)
        return user_msg;

    return rev_msg;
}

__INTR_HANDLER (intr_common)
{
    printk ("intr occurred !!! - [%03x] %s ->\n", vector, msgget (vector));
    printk ("  rax=%016llx rbx=%016llx rcx=%016llx rdx=%016llx\n", frame->rax, frame->rbx, frame->rcx, frame->rdx);
    printk ("  rsi=%016llx rdi=%016llx rbp=%016llx rsp=%016llx\n", frame->rsi, frame->rdi, frame->rbp, frame->rsp);
    printk ("  r8 =%016llx r9 =%016llx r10=%016llx r11=%016llx\n", frame->r8 , frame->r9 , frame->r10, frame->r11);
    printk ("  r12=%016llx r13=%016llx r14=%016llx r15=%016llx\n", frame->r12, frame->r13, frame->r14, frame->r15);
    printk ("  err=%016llx rip=%016llx rfl=%08llx\n", frame->rip, frame->rflags, errcode);
    printk ("<-\n");
    
    while (true) halt();
}

extern u8 intr_entries; // a start point of the whole table

ihandler_t intr_handlers[HANDLER_MAX];

void idt_init ()
{
    memset (idts, 0, HANDLER_MAX * sizeof(idt_t));

    for (size_t i = 0 ; i < HANDLER_MAX ; i++)
    {
        _IDT_SET_ENTRY (i, (u64)(&intr_entries + ENTRY_SIZ * i));
        intr_handlers[i] = (ihandler_t)intr_common;
    }

    idtr.base = (u64)&idts;
    idtr.limit = HANDLER_MAX * sizeof(idt_t) - 1;

    load_idt (&idtr);
}

void intr_register (u8 vector, ihandler_t handler)
{
    intr_handlers[vector] = handler;
}
