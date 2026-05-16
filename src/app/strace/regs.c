#include "regs.h"
#include <sys/user.h>

#ifdef __x86_64__
void collect_args(struct regs *r, struct user_regs_struct *ur)
{
    r->nr = ur->orig_rax;
    r->a0 = ur->rax;
    r->a1 = ur->rdi;
    r->a2 = ur->rsi;
    r->a3 = ur->rcx;
    r->a4 = ur->r10;
    r->a5 = ur->r8;
    r->a6 = ur->r9;
}
#endif
