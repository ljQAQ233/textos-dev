#pragma once

extern long __w_syscall(long num, ...);
extern void *__w_setval(int d, int k, void *v);
extern void *__w_getval(int d, int k);
extern void __w_delval(int d, int k);
