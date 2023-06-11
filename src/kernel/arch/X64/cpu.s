[bits 64]

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

