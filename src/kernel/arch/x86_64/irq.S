.globl intr_get

intr_get:
    pushfq
    popq %rax
    shrq $9, %rax
    andq $1, %rax
    ret
