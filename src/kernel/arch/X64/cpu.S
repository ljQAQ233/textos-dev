/* void cpuid(int leaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx); */
.globl cpuid
cpuid:
    pushq %rbx          /* 被调用方保存 */

    movq %rdi, %rax     /* leaf */
    movq %rdx, %rdi
    movq %rcx, %r9
    xorq %rcx, %rcx

    cpuid
    movl %eax, (%rsi)   /* eax */
    movl %ebx, (%rdi)   /* ebx */
    movl %ecx, (%r9)    /* ecx */
    movl %edx, (%r8)    /* edx */

    popq %rbx
    ret

/* void halt(); */
.globl halt
halt:
    hlt
    ret

/* void read_gdt(void *gdtr); */
.globl read_gdt
read_gdt:
    sgdt (%rdi)
    ret

/* void load_gdt(void *gdtr); */
.globl load_gdt
load_gdt:
    lgdt (%rdi)
    ret

/* void read_idt(void *idtr); */
.globl read_idt
read_idt:
    sidt (%rdi)
    ret

/* void load_idt(void *idtr); */
.globl load_idt
load_idt:
    lidt (%rdi)
    ret

/* void load_tss(u16 idx); */
.globl load_tss
load_tss:
    ltr %di
    ret

/* u64 read_tsc(); */
.globl read_tsc
read_tsc:
    rdtsc
    shlq $32, %rdx
    orq %rdx, %rax
    ret

/* void reload_segs(); */
.globl reload_segs
reload_segs:
    movw $0x0, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    movq %rsp, %rdx
    pushq %rdi       /* ss */
    pushq %rdx       /* rsp */
    pushfq           /* rflags */
    pushq %rsi       /* cs */
    leaq .ret(%rip), %rcx
    pushq %rcx       /* rip */

    iretq
.ret:
    ret              /* recovery rip */

/* u64 read_cr0(); */
.globl read_cr0
read_cr0:
    movq %cr0, %rax
    ret

/* void write_cr0(u64 cr0); */
.globl write_cr0
write_cr0:
    movq %rdi, %cr0
    ret

/* u64 read_cr2(); */
.globl read_cr2
read_cr2:
    movq %cr2, %rax
    ret

/* u64 read_cr3(); */
.globl read_cr3
read_cr3:
    movq %cr3, %rax
    ret

/* void write_cr3(u64 cr3); */
.globl write_cr3
write_cr3:
    movq %rdi, %cr3
    ret

/* u64 read_msr(u32 idx); */
.globl read_msr
read_msr:
    movl %edi, %ecx     /* 所以,我们使用 ecx 作为 idx */
    rdmsr
    shlq $32, %rdx      /* edx 作高位 */
    orq %rdx, %rax      /* 储存在 rax 直接成返回值 */
    ret

/* void write_msr(u32 idx, u64 value); */
.globl write_msr
write_msr:
    movl %edi, %ecx     /* MSR[ECX] := EDX:EAX */
    movl %esi, %eax
    shrq $32, %rsi
    movl %esi, %edx
    wrmsr
    ret

/* void __kstack_init(); */
.globl __kstack_init
__kstack_init:
    popq %rcx
    leaq __kstack_top(%rip), %rdx
    movq %rdx, %rbp
    movq %rdx, %rsp
    pushq %rcx
    ret

.section .data

.globl __cpu_count, __cpu_started
__cpu_count: .long 0
__cpu_started: .long 1

.align 16
.globl __kstack
__kstack:
    .zero 0x1000 * 8
__kstack_top:
