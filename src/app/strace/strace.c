//
// A very hacky tool that mimics "strace", I mean, using a pile of macros.
// Created by maouai233 on May 16, 2026. (๑•̀ㅂ•́) ✧
//

// NOTE: this implementation cannot run on TextOS
//       you could test this prog on linux!
#include "strace.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#define chkst(s) \
    if (WIFEXITED(s)) goto exited

#define MAP_MAX 512

struct scmap
{
    const char *name;
    struct type *func;
};

#define _(sc) [SYS_##sc] = {_STR(sc), reffun(sc)},
struct scmap map[MAP_MAX] = {SYSCALLS XSYSCALLS};
#undef _

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        fprintf(stderr, "%s: prog [...]\n", argv[0]);
        return 1;
    }
    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], &argv[1]);
        _Exit(1);
    }
    int status;
    struct type *fun;
    struct regs regs;
    struct user_regs_struct uregs, uregs_out;
    wait(&status);
    chkst(status);
    for (;;) {
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);
        // syscall entry
        chkst(status);
        ptrace(PTRACE_GETREGS, pid, NULL, &uregs);

        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        wait(&status);
        // syscall exit
        chkst(status);
        ptrace(PTRACE_GETREGS, pid, NULL, &uregs_out);
        collect_args(&regs, &uregs, &uregs_out);

        // summarize
        fun = 0;
        if (regs.nr < MAP_MAX)
            fun = map[regs.nr].func;
        else
            fprintf(stderr, "unknown syscall - %ld\n", (long)regs.nr);
        if (fun == 0)
            fprintf(stderr, "syscall(%d, %lx, %lx, %lx, %lx, %lx, %lx) = %d\n",
                    (int)regs.nr, regs.a1, regs.a2, regs.a3, regs.a4, regs.a5,
                    regs.a6, regs.ret);
        else
            fun->printer(stderr, fun, &regs);
    }
    return 0;
exited:
    fprintf(stderr, "child exited with code %d\n", WEXITSTATUS(status));
    return 0;
}
