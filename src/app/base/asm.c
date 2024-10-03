#include <app/api.h>
#include <app/sys.h>
#include <app/args.h>

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
    return a0;
}

int fork()
{
    return syscall(SYS_fork);
}

int execve(char *path, char *const argv[], char *const envp[])
{
    return syscall(SYS_execve, path, argv, envp);
}

ssize_t write(int fd, const void *buf, size_t cnt)
{
    return syscall(SYS_write, fd, buf, cnt);
}

ssize_t read(int fd, void *buf, size_t cnt)
{
    return syscall(SYS_read, fd, buf, cnt);
}

int close(int fd)
{
    return syscall(SYS_close, fd);
}

int getpid()
{
    return syscall(SYS_getpid);
}

int getppid()
{
    return syscall(SYS_getppid);
}
