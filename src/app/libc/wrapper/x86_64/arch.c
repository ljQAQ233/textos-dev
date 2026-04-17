#include <stdarg.h>
#include <bits/a-sc.h>
#include <bits/syscall.h>

static long w_syscall(long num, ...)
{
    va_list ap;
    va_start(ap, num);
    __sc_type a0 = num;
    __sc_type a1 = va_arg(ap, __sc_type);
    __sc_type a2 = va_arg(ap, __sc_type);
    __sc_type a3 = va_arg(ap, __sc_type);
    __sc_type a4 = va_arg(ap, __sc_type);
    __sc_type a5 = va_arg(ap, __sc_type);
    __sc_type a6 = va_arg(ap, __sc_type);
    va_end(ap);

    return __syscall(a0, a1, a2, a3, a4, a5, a6);
}

#define __wrapper
#include "sig.c"
#include "stat.c"

void *__sc_redir[512] = {
    [SYS_sigaction] = w_sigaction,
    [SYS_stat] = w_stat,
    [SYS_fstat] = w_fstat,
};

void __arch_init_wrapper()
{
    (void)w_syscall;
}
