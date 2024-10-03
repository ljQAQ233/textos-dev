#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define _SYSCALL         \
    _(read, 0),          \
    _(write, 1),         \
    _(open, 2),          \
    _(close, 3),         \
    _(execve, 11),       \
    _(getpid, 20),       \
    _(fork, 57),         \
    _(getppid, 64),      \
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

extern void msyscall_exit();

#endif
