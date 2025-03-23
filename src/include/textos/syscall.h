#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define _SYSCALL         \
    _(read, 0),          \
    _(write, 1),         \
    _(open, 2),          \
    _(close, 3),         \
    _(stat, 4),          \
    _(ioctl, 16),        \
    _(pipe, 22),         \
    _(dup, 32),          \
    _(dup2, 33),         \
    _(getpid, 39),       \
    _(socket, 41),       \
    _(connect, 42),      \
    _(sendto, 44),       \
    _(recvfrom, 45),     \
    _(sendmsg, 46),      \
    _(recvmsg, 47),      \
    _(bind, 49),         \
    _(getsockname, 51),  \
    _(getpeername, 52),  \
    _(fork, 57),         \
    _(execve, 59),       \
    _(exit, 60),         \
    _(wait4, 61),        \
    _(chdir, 80),        \
    _(mkdir, 83),        \
    _(rmdir, 84),        \
    _(getppid, 110),     \
    _(mknod, 133),       \
    _(mount, 165),       \
    _(readdir, 500),     \
    _(poweroff, 501),    \
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

#define RETVAL(x) long

extern void msyscall_exit();

#endif
