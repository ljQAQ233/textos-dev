#include <app/api.h>
#include <app/sys.h>

#include <stdarg.h>

extern int errno;

// According to the SysV ABI :
// Returning from the syscall, register %rax contains the result of the system-call. A
// value in the range between -4095 and -1 indicates an error, it is -errno.
static long syscall_ret(long ret)
{
    if ((unsigned long)ret > -4096UL)
    {
        errno = -ret;
        return -1;
    }
    return ret;
}

long syscall(int num, ...)
{
    va_list ap;
    va_start(ap, num);
    register long a0 asm ("rax") = num;
    register long a1 asm ("rdi") = va_arg(ap, long);
    register long a2 asm ("rsi") = va_arg(ap, long);
    register long a3 asm ("rdx") = va_arg(ap, long);
    register long a4 asm ("r10") = va_arg(ap, long);
    register long a5 asm ("r8") = va_arg(ap, long);
    register long a6 asm ("r9") = va_arg(ap, long);
    va_end(ap);
    asm volatile(
        "syscall"
        : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6)
        : "memory");
    return syscall_ret(a0);
}

int fork()
{
    return syscall(SYS_fork);
}

int execve(char *path, char *const argv[], char *const envp[])
{
    return syscall(SYS_execve, path, argv, envp);
}

void _exit(int stat)
{
    syscall(SYS_exit, stat);
}

int wait4(int pid, int *stat, int opt, void *rusage)
{
    return syscall(SYS_wait4, pid, stat, opt, rusage);
}

int wait(int *stat)
{
    return wait4(-1, stat, 0, NULL);
}

ssize_t write(int fd, const void *buf, size_t cnt)
{
    return syscall(SYS_write, fd, buf, cnt);
}

int open(char *path, int flgs)
{
    return syscall(SYS_open, path, flgs);
}

ssize_t read(int fd, void *buf, size_t cnt)
{
    return syscall(SYS_read, fd, buf, cnt);
}

int close(int fd)
{
    return syscall(SYS_close, fd);
}

int dup(int fd)
{
    return syscall(SYS_dup, fd);
}

int dup2(int old, int new)
{
    return syscall(SYS_dup2, old, new);
}

int pipe(int fds[2])
{
    return syscall(SYS_pipe, fds);
}

int getpid()
{
    return syscall(SYS_getpid);
}

int getppid()
{
    return syscall(SYS_getppid);
}
