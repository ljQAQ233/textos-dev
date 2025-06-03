#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define _SYSCALL         \
    _(read, 0),          \
    _(write, 1),         \
    _(open, 2),          \
    _(close, 3),         \
    _(stat, 4),          \
    _(mmap, 9),          \
    _(mprotect, 10),     \
    _(munmap, 11),       \
    _(sigaction, 13),    \
    _(sigprocmask, 14),  \
    _(sigreturn, 15),    \
    _(ioctl, 16),        \
    _(pipe, 22),         \
    _(dup, 32),          \
    _(dup2, 33),         \
    _(getpid, 39),       \
    _(socket, 41),       \
    _(connect, 42),      \
    _(accept, 43),       \
    _(sendto, 44),       \
    _(recvfrom, 45),     \
    _(sendmsg, 46),      \
    _(recvmsg, 47),      \
    _(shutdown, 48),     \
    _(bind, 49),         \
    _(listen, 50),       \
    _(getsockname, 51),  \
    _(getpeername, 52),  \
    _(fork, 57),         \
    _(execve, 59),       \
    _(exit, 60),         \
    _(wait4, 61),        \
    _(kill, 62),         \
    _(uname, 63),        \
    _(chdir, 80),        \
    _(mkdir, 83),        \
    _(rmdir, 84),        \
    _(getppid, 110),     \
    _(mknod, 133),       \
    _(mount, 165),       \
    _(umount2, 166),     \
    _(readdir, 500),     \
    _(poweroff, 501),    \
    _(yield, 502),       \
    _(seekdir, 503),     \
    _(test, 510),        \
    _(maxium, 511),

#define _(name, num) SYS_##name = num
enum sys_syscall
{
    _SYSCALL
};
#undef _

#define _(name, num) __NR_##name = num
enum nr_syscall
{
    _SYSCALL
};
#undef _

#undef _SYSCALL

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
