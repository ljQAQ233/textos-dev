#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include <bits/syscall.h>

#define RETVAL(x) long long

#define __SC_DECL(t, a) t a // type arg
#define __SC_ARGS(t, a) a   //      arg

#define __MAP0(m, ...)
#define __MAP1(m, t, a, ...) m(t, a)
#define __MAP2(m, t, a, ...) m(t, a), __MAP1(m, __VA_ARGS__)
#define __MAP3(m, t, a, ...) m(t, a), __MAP2(m, __VA_ARGS__)
#define __MAP4(m, t, a, ...) m(t, a), __MAP3(m, __VA_ARGS__)
#define __MAP5(m, t, a, ...) m(t, a), __MAP4(m, __VA_ARGS__)
#define __MAP6(m, t, a, ...) m(t, a), __MAP5(m, __VA_ARGS__)
#define __MAP(n, ...) __MAP##n(__VA_ARGS__)

#define __SYSCALL_DEFINEx(n, type, name, ...)            \
type name(__MAP(n, __SC_DECL, __VA_ARGS__));      \
RETVAL() sys_##name(__MAP(n, __SC_DECL, __VA_ARGS__)) {  \
        return (RETVAL())name(__MAP(n, __SC_ARGS, __VA_ARGS__));   \
    }                                                    \
type name(__MAP(n, __SC_DECL, __VA_ARGS__))

#define __SYSCALL_DEFINE0(type, name, ...) __SYSCALL_DEFINEx(0, type, name, ##__VA_ARGS__)
#define __SYSCALL_DEFINE1(type, name, ...) __SYSCALL_DEFINEx(1, type, name, ##__VA_ARGS__)
#define __SYSCALL_DEFINE2(type, name, ...) __SYSCALL_DEFINEx(2, type, name, ##__VA_ARGS__)
#define __SYSCALL_DEFINE3(type, name, ...) __SYSCALL_DEFINEx(3, type, name, ##__VA_ARGS__)
#define __SYSCALL_DEFINE4(type, name, ...) __SYSCALL_DEFINEx(4, type, name, ##__VA_ARGS__)
#define __SYSCALL_DEFINE5(type, name, ...) __SYSCALL_DEFINEx(5, type, name, ##__VA_ARGS__)
#define __SYSCALL_DEFINE6(type, name, ...) __SYSCALL_DEFINEx(6, type, name, ##__VA_ARGS__)

extern void msyscall_exit();

#endif
