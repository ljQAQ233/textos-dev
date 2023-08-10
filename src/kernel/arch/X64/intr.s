[bits 64]

%define HANDLER_MAX 256
%define ENTRY_SIZ   16

extern intr_handlers

section .text

%macro ENTRY 2
section .text

intr_handler%1:
    ; SS -> RSP (original RSP) -> RFLAGS -> CS -> RIP
    ; - 1 - vector
    ; - 2 - errorcode / reserved only
    %%head:
    %ifn %2
      push QWORD 2333
    %endif
    push QWORD %1
    jmp  intr_caller

    times ENTRY_SIZ - ($ - %%head) db 0 ; Zero to fill

%endmacro

global intr_exit

intr_caller:
    push  rax ; 保存寄存器 & 为后做铺垫
    push  rbx
    push  rcx
    push  rdx
    push  rbp
    push  rsi
    push  rdi
    push  r8
    push  r9
    push  r10
    push  r11
    push  r12
    push  r13
    push  r14
    push  r15

    mov  rdi, [rsp + 15 * 8] ; vector
    mov  rsi, [rsp + 16 * 8] ; errcode
    mov  rdx, rsp            ; intr context

    mov  rax, intr_handlers
    lea  rax, [rax + rdi * 8]
    call [rax]

intr_exit:
    pop  r15 ; 恢复寄存器
    pop  r14
    pop  r13
    pop  r12
    pop  r11
    pop  r10
    pop  r9
    pop  r8
    pop  rdi
    pop  rsi
    pop  rbp
    pop  rdx
    pop  rcx
    pop  rbx
    pop  rax

    add     rsp, 16        ; 修正栈指针，跳过 Vector 和 ErrorCode
    iretq                  ; 中断返回

global intr_entries
intr_entries:
    ENTRY 0,  0
    ENTRY 1,  0
    ENTRY 2,  0
    ENTRY 3,  0
    ENTRY 4,  0
    ENTRY 5,  0
    ENTRY 6,  0
    ENTRY 7,  0
    ENTRY 8,  1
    ENTRY 9,  0
    ENTRY 10, 1
    ENTRY 11, 1
    ENTRY 12, 1
    ENTRY 13, 1
    ENTRY 14, 1
    ENTRY 15, 0
    ENTRY 16, 0
    ENTRY 17, 1
    ENTRY 18, 0
    ENTRY 19, 0
    ENTRY 20, 0
    ENTRY 21, 1
    ENTRY 22, 0
    ENTRY 23, 0
    ENTRY 24, 0
    ENTRY 25, 0
    ENTRY 26, 0
    ENTRY 27, 0
    ENTRY 28, 0
    ENTRY 29, 0
    ENTRY 30, 0
    ENTRY 31, 0

    %assign i 32
    %rep HANDLER_MAX - 32
        ENTRY i, 0
        %assign i (i + 1)
    %endrep
    
