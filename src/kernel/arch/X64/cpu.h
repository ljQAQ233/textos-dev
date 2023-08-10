#ifndef __CPU_H__
#define __CPU_H__

typedef struct {
  u64 r15, r14, r13, r12, r11, r10, r9, r8;
  u64 rdi, rsi;
  u64 rbp;
  u64 rdx, rcx, rbx, rax;

  u64 errcode;
  u64 vector;

  u64 rip;
  u64 cs;
  u64 rflags;
  u64 rsp;
  u64 ss;
} intr_frame_t;

void cpuid (int leaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

void halt ();

void read_get (void *gdtr);

void load_gdt (void *gdtr);

void reload_segs (u64 ss, u64 cs);

void read_idt (void *Idtr);

void load_idt (void *Idtr);

u64 read_cr3 ();

u64 write_cr3 ();

#define IA32_APIC_BASE 0x1B

u64 read_msr (u32 idx);

void write_msr (u32 idx, u64 value);

#endif

