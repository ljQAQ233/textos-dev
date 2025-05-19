extern sys_handlers
extern syscall_handler
extern task_current

section .text

%define TASK_ISTACK 0x00
%define TASK_IFRAME 0x10
%define TASK_SFRMAE 0x18
%define TASK_OLDSP  0x20

global msyscall_exit

global msyscall_handler
msyscall_handler:
    mov  [gs:TASK_OLDSP], rsp
    mov  rsp, [gs:TASK_ISTACK] ; istk

    push qword 0x23 ; user ss
    push qword [gs:TASK_OLDSP] ; rsp
    push r11        ; rflags
    push qword 0x2b ; user cs
    push rcx        ; rip
    push qword 2333 ; errcode
    push qword 0x81 ; vector

    push rax
    push rbx
    push qword 0 ; old rip
    push rdx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push qword 0 ; old eflags
    push r12
    push r13
    push r14
    push r15

    mov  rdi, [rsp + 15 * 8]
    mov  rsi, [rsp + 16 * 8]
    mov  rdx, rsp
    call syscall_handler

msyscall_exit:
    pop  r15
    pop  r14
    pop  r13
    pop  r12
    add  rsp, 8
    pop  r10
    pop  r9
    pop  r8
    pop  rdi
    pop  rsi
    pop  rbp
    pop  rdx
    add  rsp, 8
    pop  rbx
    pop  rax

    ; 涉及到 用户堆栈 的恢复, 如果发生 定时器中断
    ; 则会直接使用 用户堆栈, 但是现在位于 ring0
    ; 所以 并不会触发 栈切换, 索性就关掉好了
    ; r11 寄存器的值在 sysret 时成为 rflags
    cli

    add  rsp, 16
    pop  rcx
    add  rsp, 8
    pop  r11
    pop  rsp

    o64 sysret
