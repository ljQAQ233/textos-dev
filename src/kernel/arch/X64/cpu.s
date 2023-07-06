[bits 64]

; void halt ();
global halt
halt:
    hlt
    ret

; void read_gdt (void *gdtr);
global read_gdt
read_gdt:
    sgdt [rdi]
    ret

; void load_gdt (void *gdtr);
global load_gdt
load_gdt:
    lgdt [rdi]
    ret

; void read_idt (void *idtr);
global read_idt
read_idt:
    sidt [rdi]
    ret

; void load_idt (void *idtr);
global load_idt
load_idt:
    lidt [rdi]
    ret

; void reload_segs ();
global reload_segs
reload_segs:
  mov ax, 0x00
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax

  ; mov  rcx, [rsp]
  mov  rdx, rsp
  push rdi      ; ss
  push rdx      ; rsp
  pushf         ; rflags
  push rsi      ; cs
  lea  rcx, [rel .ret]
  push rcx      ; rip

  iretq
  .ret: ; iretq jump here
    ret ; recovery rip

; u64 read_cr3 ();
global read_cr3
read_cr3:
    mov rax, cr3
    ret

; void write_cr3 ();
global write_cr3
write_cr3:
    mov cr3, rdi
    ret

; void __kstack_init ();
global __kstack_init
__kstack_init:
    pop  rbx
    lea  rax, [rel __kstack_top]
    mov  rbp, rax
    mov  rsp, rax
    push rbx
    ret

section .data

global __kstack:
    times 0x1000 dq 1
    __kstack_top:

