#pragma once

#if !defined(__textos__) && defined(__linux__)
    #define SYS_sigprocmask SYS_rt_sigprocmask
    #define SYS_sigreturn   SYS_rt_sigreturn
    #define SYS_sigaction   SYS_rt_sigaction
#endif

#define SYSCALLS    \
    _(read)         \
    _(write)        \
    _(open)         \
    _(close)        \
    _(stat)         \
    _(fstat)        \
    _(lseek)        \
    _(mmap)         \
    _(mprotect)     \
    _(munmap)       \
    _(brk)          \
    _(sigaction)    \
    _(sigprocmask)  \
    _(sigreturn)    \
    _(ioctl)        \
    _(readv)        \
    _(writev)       \
    _(access)       \
    _(pipe)         \
    _(dup)          \
    _(dup2)         \
    _(pause)        \
    _(nanosleep)    \
    _(getpid)       \
    _(socket)       \
    _(connect)      \
    _(accept)       \
    _(sendto)       \
    _(recvfrom)     \
    _(sendmsg)      \
    _(recvmsg)      \
    _(shutdown)     \
    _(bind)         \
    _(listen)       \
    _(getsockname)  \
    _(getpeername)  \
    _(fork)         \
    _(execve)       \
    _(exit)         \
    _(wait4)        \
    _(kill)         \
    _(uname)        \
    _(fcntl)        \
    _(getcwd)       \
    _(chdir)        \
    _(mkdir)        \
    _(rmdir)        \
    _(chmod)        \
    _(fchmod)       \
    _(chown)        \
    _(fchown)       \
    _(gettimeofday) \
    _(times)        \
    _(getuid)       \
    _(getgid)       \
    _(setuid)       \
    _(setgid)       \
    _(geteuid)      \
    _(getegid)      \
    _(setpgid)      \
    _(getppid)      \
    _(setsid)       \
    _(setreuid)     \
    _(setregid)     \
    _(getgroups)    \
    _(setgroups)    \
    _(getpgid)      \
    _(getsid)       \
    _(mknod)        \
    _(mount)        \
    _(umount2)      \
    _(sethostname)  \
    _(time)

#if !defined(__textos__)
    #define XSYSCALLS
#else
    #define XSYSCALLS \
        _(readdir)    \
        _(poweroff)   \
        _(yield)      \
        _(seekdir)    \
        _(test)       \
        _(maximum)
#endif
