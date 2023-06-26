
; u8 inb (u16 port);
global inb
inb:
    xor rax, rax
    mov dx,  di
    in  al,  dx
    ret

; u16 inw (u16 port);
global inw
inw:
    xor rax, rax
    mov dx,  di
    in  ax,  dx
    ret

; u32 indw (u16 port);
global indw
indw:
    xor rax, rax
    mov dx,  di
    in  eax, dx
    ret

; void outb (u16 port, u8 data);
global outb
outb:
    mov dx, di
    mov al, sil
    out dx, al
    ret

; void outw (u16 port, u16 data);
global outw
outw:
    mov dx, di
    mov ax, si
    out dx, ax
    ret

; void outdw (u16 port, u32 data);
global outdw
outdw:
    mov dx,  di
    mov eax, esi
    out dx,  eax
    ret
