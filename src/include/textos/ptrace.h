#ifndef __PTRACE_H__
#define __PTRACE_H__

#include <bits/user.h>
#include <bits/ptrace.h>

#include <cpu.h> // struct intr_frame

void arch_ptrace_to_regs(struct user_regs_struct *uregs,
                         struct intr_frame *regs, unsigned long *syscallno);
void arch_ptrace_to_uregs(struct user_regs_struct *uregs,
                          struct intr_frame *regs, unsigned long syscallno);

/* linked with PTRACE_O_* */
#define PTRACE_EVENT_FORK       1 // TODO
#define PTRACE_EVENT_VFORK      2 // TODO
#define PTRACE_EVENT_CLONE      3 // TODO
#define PTRACE_EVENT_EXEC       4 // TODO
#define PTRACE_EVENT_VFORK_DONE 5 // TODO
#define PTRACE_EVENT_EXIT       6 // TODO
#define PTRACE_EVENT_SECCOMP    7 // TODO

/* textos-specific events */
#define PTRACE_EVENT_SCENTRY 16
#define PTRACE_EVENT_SCEXIT  17

#define PTRACE_F_SCENTRY (1 << PTRACE_EVENT_SCENTRY)
#define PTRACE_F_SCEXIT  (1 << PTRACE_EVENT_SCEXIT)

void ptrace_event(int ev);

#endif
