#define _SYSCALL         \
    _(read, 0)          \
    _(write, 1)         \
    _(open, 2)          \
    _(close, 3)         \
    _(stat, 4)          \
    _(fstat, 5)         \
    _(lseek, 8)         \
    _(mmap, 9)          \
    _(mprotect, 10)     \
    _(munmap, 11)       \
    _(brk, 12)          \
    _(sigaction, 13)    \
    _(sigprocmask, 14)  \
    _(sigreturn, 15)    \
    _(ioctl, 16)        \
    _(readv, 19)        \
    _(writev, 20)       \
    _(access, 21)       \
    _(pipe, 22)         \
    _(dup, 32)          \
    _(dup2, 33)         \
    _(nanosleep, 35)    \
    _(getpid, 39)       \
    _(socket, 41)       \
    _(connect, 42)      \
    _(accept, 43)       \
    _(sendto, 44)       \
    _(recvfrom, 45)     \
    _(sendmsg, 46)      \
    _(recvmsg, 47)      \
    _(shutdown, 48)     \
    _(bind, 49)         \
    _(listen, 50)       \
    _(getsockname, 51)  \
    _(getpeername, 52)  \
    _(fork, 57)         \
    _(execve, 59)       \
    _(exit, 60)         \
    _(wait4, 61)        \
    _(kill, 62)         \
    _(uname, 63)        \
    _(fcntl, 72)        \
    _(getcwd, 79)       \
    _(chdir, 80)        \
    _(mkdir, 83)        \
    _(rmdir, 84)        \
    _(chmod, 90)        \
    _(fchmod, 91)       \
    _(chown, 92)        \
    _(fchown, 93)       \
    _(gettimeofday, 96) \
    _(getuid, 102)      \
    _(getgid, 104)      \
    _(setuid, 105)      \
    _(setgid, 106)      \
    _(geteuid, 107)     \
    _(getegid, 108)     \
    _(setpgid, 109)     \
    _(getppid, 110)     \
    _(setsid, 112)      \
    _(setreuid, 113)    \
    _(setregid, 114)    \
    _(getgroups, 115)   \
    _(setgroups, 116)   \
    _(getpgid, 121)     \
    _(getsid, 124)      \
    _(mknod, 133)       \
    _(mount, 165)       \
    _(umount2, 166)     \
    _(sethostname, 170) \
    _(time, 201)        \
    _(readdir, 500)     \
    _(poweroff, 501)    \
    _(yield, 502)       \
    _(seekdir, 503)     \
    _(test, 510)        \
    _(maxium, 511)

#define _(name, num) SYS_##name = num,
enum sys_syscall
{
    _SYSCALL
};
#undef _

#define _(name, num) __NR_##name = num,
enum nr_syscall
{
    _SYSCALL
};
#undef _
