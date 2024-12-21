; void fninit();
global fninit
fninit:
    fninit
    ret

; void fnclex();
global fnclex
fnclex:
    fnclex
    ret

; void fldcw(u16 *ctrl);
global fldcw
fldcw:
    fldcw [rdi]
    ret

; void fnstcw(u16 *ctrl);
global fnstcw
fnstcw:
    fnstcw [rdi]
    ret

; void fnstsw(u16 *sreg);
global fnstsw
fnstsw:
    fnstsw [rdi]
    ret

; void fnsave(void *stat);
global fnsave
fnsave:
    fnsave [rdi]
    ret

; void frstor(void *stat);
global frstor
frstor:
    frstor [rdi]

