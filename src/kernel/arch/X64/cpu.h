#ifndef __CPU_H__
#define __CPU_H__

typedef struct {
  u64 r15, r14, r13, r12, r11, r10, r9, r8;
  u64 rdi, rsi;
  u64 rbp;
  u64 rdx, rcx, rbx, rax;

  u64 vector;
  u64 errcode;

  u64 rip;
  u64 cs;
  u64 rflags;
  u64 rsp;
  u64 ss;
} intr_frame_t;

typedef struct {
  void *restorer;
  u64 signum;
  u64 sigmask;

  u64 r15, r14, r13, r12, r11, r10, r9, r8;
  u64 rdi, rsi;
  u64 rbp;
  u64 rdx, rcx, rbx, rax;

  u64 vector;
  u64 errcode;

  u64 rip;
  u64 cs;
  u64 rflags;
  u64 rsp;
  u64 ss;
} signal_frame_t;

void cpuid (int leaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);

void halt ();

void read_get (void *gdtr);

void load_gdt (void *gdtr);

void reload_segs (u64 ss, u64 cs);

void read_idt (void *idtr);

void load_idt (void *idtr);

void load_tss (u16 idx);

u64 read_tsc ();

#define CR0_PE  (1 << 0)
#define CR0_MP  (1 << 1)
#define CR0_EM  (1 << 2)
#define CR0_TS  (1 << 3)
#define CR0_ET  (1 << 4)
#define CR0_NE  (1 << 5)
#define CR0_WP  (1 << 16)
#define CR0_AM  (1 << 18)
#define CR0_NW  (1 << 29)
#define CR0_CD  (1 << 30)
#define CR0_PG  (1 << 31)

u64 read_cr0 ();

u64 write_cr0 (u64 cr0);

u64 read_cr3 ();

u64 write_cr3 (u64 cr3);

#define MSR_EFER   0xC0000080
#define MSR_STAR   0xC0000081 // eip / seg
#define MSR_LSTAR  0xC0000082 // rip - long mode
#define MSR_CSTAR  0xC0000083 // rip - compact
#define MSR_FMASK  0xC0000084

#define MSR_FS_BASE 0xc0000100
#define MSR_GS_BASE 0xc0000101

#define IA32_APIC_BASE 0x1B

u64 read_msr (u32 idx);

void write_msr (u32 idx, u64 value);

#endif

