[bits 64]

; void cpuid (int leaf, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
global cpuid
cpuid:
  push rbx ; 被调用方保存
  
  mov rax, rdi          ; leaf
  cpuid
  mov dword [rsi], eax  ; eax
  mov dword [rdx], ebx  ; ebx
  mov dword [rcx], ecx  ; ecx
  mov dword [r8],  edx  ; edx
  
  pop  rbx
  ret

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

; void load_tss (u16 idx);
global load_tss
load_tss:
    ltr di
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

; void write_cr3 (u64 cr3);
global write_cr3
write_cr3:
    mov cr3, rdi
    ret

; u64 read_msr (u32 idx);
global read_msr
read_msr:           ; EDX:EAX := MSR[ECX]
    mov ecx, edi   ; 所以,我们使用 ecx 作为 idx
    rdmsr
    shl rdx, 32    ; edx 作高位
    or  rax, rdx   ; 储存在 rax 直接成返回值
    ret

; void write_msr (u32 idx, u64 value);
global write_msr
write_msr:
    mov ecx, edi   ; MSR[ECX] := EDX:EAX
    mov eax, esi
    shr rsi, 32
    mov edx, esi
    wrmsr
    ret

; void __kstack_init ();
global __kstack_init
__kstack_init:
    pop  rcx
    lea  rdx, [rel __kstack_top]
    mov  rbp, rdx
    mov  rsp, rdx
    push rcx
    ret

section .data

global __kstack:
    times 0x1000 dq 1
    __kstack_top:

