.globl fninit
.globl fnclex
.globl fldcw
.globl fnstcw
.globl fnstsw
.globl fnsave
.globl frstor

/* void fninit(); */
fninit:
    fninit
    ret

/* void fnclex(); */
fnclex:
    fnclex
    ret

/* void fldcw(u16 *ctrl); */
fldcw:
    fldcw   (%rdi)
    ret

/* void fnstcw(u16 *ctrl); */
fnstcw:
    fnstcw  (%rdi)
    ret

/* void fnstsw(u16 *sreg); */
fnstsw:
    fnstsw  (%rdi)
    ret

/* void fnsave(void *stat); */
fnsave:
    fnsave  (%rdi)
    ret

/* void frstor(void *stat); */
frstor:
    frstor  (%rdi)
    ret

