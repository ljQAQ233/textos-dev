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

int uname(utsname_t *name)
{
    return syscall(SYS_uname, name);
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

ssize_t readdir(int fd, void *buf, size_t mx)
{
    return syscall(SYS_readdir, fd, buf, mx);
}

int close(int fd)
{
    return syscall(SYS_close, fd);
}

int stat(char *path, stat_t *sb)
{
    return syscall(SYS_stat, path, sb);
}

int ioctl(int fd, int req, ...)
{
    va_list args;
    va_start(args, req);
    void *argp = va_arg(args, void *);
    va_end(args);

    return syscall(SYS_ioctl, fd, req, argp);
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

int mknod(char *path, int mode, long dev)
{
    return syscall(SYS_mknod, path, mode, dev);
}

int mount(char *src, char *dst)
{
    return syscall(SYS_mount, src, dst);
}

int umount2(char *target, int flags)
{
    return syscall(SYS_umount2, target, flags);
}

int umount(char *target)
{
    return umount2(target, 0);
}

int chdir(char *path)
{
    return syscall(SYS_chdir, path);
}

int mkdir(char *path, int mode)
{
    return syscall(SYS_mkdir, path, mode);
}

int rmdir(char *path)
{
    return syscall(SYS_rmdir, path);
}

int socket(int domain, int type, int proto)
{
    return syscall(SYS_socket, domain, type, proto);
}

int bind(int fd, sockaddr_t *addr, size_t len)
{
    return syscall(SYS_bind, fd, addr, len);
}

int listen(int fd, int backlog)
{
    return syscall(SYS_listen, fd, backlog);
}

int accept(int fd, sockaddr_t *addr, size_t *len)
{
    return syscall(SYS_accept, fd, addr, len);
}

int connect(int fd, sockaddr_t *addr, size_t len)
{
    return syscall(SYS_connect, fd, addr, len);
}

int shutdown(int fd, int how)
{
    return syscall(SYS_shutdown, fd, how);
}

int getsockname(int fd, sockaddr_t *addr, size_t len)
{
    return syscall(SYS_getsockname, fd, addr, len);
}

int getpeername(int fd, sockaddr_t *addr, size_t len)
{
    return syscall(SYS_getpeername, fd, addr, len);
}

ssize_t sendmsg(int fd, msghdr_t *msg, u32 flags)
{
    return syscall(SYS_sendmsg, fd, msg, flags);
}

ssize_t recvmsg(int fd, msghdr_t *msg, u32 flags)
{
    return syscall(SYS_recvmsg, fd, msg, flags);
}

ssize_t sendto(int fd, void *buf, size_t len, int flags, sockaddr_t *dst, size_t dlen)
{
    return syscall(SYS_sendto, fd, buf, len, flags, dst, dlen);
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, sockaddr_t *src, size_t slen)
{
    return syscall(SYS_recvfrom, fd, buf, len, flags, src, slen);
}

ssize_t send(int fd, void *buf, size_t len, int flags)
{
    return sendto(fd, buf, len, flags, NULL, 0);
}

ssize_t recv(int fd, void *buf, size_t len, int flags)
{
    return recvfrom(fd, buf, len, flags, NULL, 0);
}

void *mmap(void *addr, size_t len, int prot, int flgs, int fd, size_t off)
{
    return (void *)syscall(SYS_mmap, addr, len, prot, flgs, fd, off);
}

int mprotect(void *addr, size_t len, int prot)
{
    return syscall(SYS_mprotect, addr, len, prot);
}

int munmap(void *addr, size_t len)
{
    return syscall(SYS_munmap, addr, len);
}

int getpid()
{
    return syscall(SYS_getpid);
}

int getppid()
{
    return syscall(SYS_getppid);
}
