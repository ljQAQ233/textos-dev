[bits 64]

global __task_switch
__task_switch:
    push rbp
    mov  rbp, rsp
    
    push  rbx
    push  r12
    push  r13
    push  r14
    push  r15
    
    mov  [rsi], rsp
    mov  rsp, rdi
    
    pop  r15
    pop  r14
    pop  r13
    pop  r12
    pop  rbx

    pop  rbp
    ret

