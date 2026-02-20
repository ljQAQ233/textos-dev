.globl inb, inw, indw
.globl outb, outw, outdw

/* u8 inb(u16 port) */
inb:
    xorq %rax, %rax
    movw %di, %dx
    inb %dx, %al
    ret

/* u16 inw(u16 port) */
inw:
    xorq %rax, %rax
    movw %di, %dx
    inw %dx, %ax
    ret

/* u32 indw(u16 port) */
indw:
    xorq %rax, %rax
    movw %di, %dx
    inl %dx, %eax
    ret

/* void outb(u16 port, u8 data) */
outb:
    movw %di, %dx
    movb %sil, %al
    outb %al, %dx
    ret

/* void outw(u16 port, u16 data) */
outw:
    movw %di, %dx
    movw %si, %ax
    outw %ax, %dx
    ret

/* void outdw(u16 port, u32 data) */
outdw:
    movw %di, %dx
    movl %esi, %eax
    outl %eax, %dx
    ret
