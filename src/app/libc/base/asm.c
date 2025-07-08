#include <stddef.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/syscall.h>

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

__attribute((noreturn))
void _exit(int stat)
{
    syscall(SYS_exit, stat);
    __builtin_unreachable();
}

int wait4(int pid, int *stat, int opt, void *rusage)
{
    return syscall(SYS_wait4, pid, stat, opt, rusage);
}

int wait(int *stat)
{
    return wait4(-1, stat, 0, NULL);
}

__attribute__((naked))
__attribute__((noreturn))
void __restorer()
{
    asm volatile(
        "syscall\n"
        :
        : "a"(15)
        : "rcx", "r11", "memory"
        );
}

void (*signal(int signum, void (*handler)(int)))(int)
{
    struct sigaction act = {
        .sa_handler = handler,
        .sa_flags = SA_RESTART,
        .sa_mask = 0,
    }, old;
    if (sigaction(signum, &act, &old) < 0)
        return SIG_ERR;
    return old.sa_handler;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
    struct sigaction __act = {
        .sa_handler = act->sa_handler,
        .sa_flags = act->sa_flags | SA_RESTORER,
        .sa_mask = act->sa_mask,
        .sa_restorer = __restorer
    };
    return syscall(SYS_sigaction, signum, &__act, oldact);
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    return syscall(SYS_sigprocmask, how, set, oset);
}

int kill(int pid, int sig)
{
    return syscall(SYS_kill, pid, sig);
}

int raise(int sig)
{
    return kill(getpid(), sig);
}

int uname(struct utsname *name)
{
    return syscall(SYS_uname, name);
}

int open(char *path, int flgs, ...)
{
    int mode = 0;
    if (flgs & O_CREAT)
    {
        va_list ap;
        va_start(ap, flgs);
        mode = va_arg(ap, int);
        va_end(ap);
    }
    return syscall(SYS_open, path, flgs, mode);
}

ssize_t read(int fd, void *buf, size_t cnt)
{
    return syscall(SYS_read, fd, buf, cnt);
}

ssize_t write(int fd, const void *buf, size_t cnt)
{
    return syscall(SYS_write, fd, buf, cnt);
}

off_t lseek(int fd, off_t off, int whence)
{
    return syscall(SYS_lseek, fd, off, whence);
}

ssize_t __readdir(int fd, void *buf, size_t mx)
{
    return syscall(SYS_readdir, fd, buf, mx);
}

int __seekdir(int fd, size_t *pos)
{
    return syscall(SYS_seekdir, fd, pos);
}

int close(int fd)
{
    return syscall(SYS_close, fd);
}

int stat(char *path, struct stat *sb)
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

int dup2(int oldfd, int newfd)
{
    return syscall(SYS_dup2, oldfd, newfd);
}

int pipe(int fds[2])
{
    return syscall(SYS_pipe, fds);
}

int mknod(char *path, mode_t mode, dev_t dev)
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

int mkdir(char *path, mode_t mode)
{
    return syscall(SYS_mkdir, path, mode);
}

int rmdir(char *path)
{
    return syscall(SYS_rmdir, path);
}

char *getcwd(char *buf, size_t size)
{
    // syscall_ret will handle errno
    return (char *)syscall(SYS_getcwd, buf, size);
}

int socket(int domain, int type, int proto)
{
    return syscall(SYS_socket, domain, type, proto);
}

int bind(int fd, struct sockaddr *addr, socklen_t len)
{
    return syscall(SYS_bind, fd, addr, len);
}

int listen(int fd, int backlog)
{
    return syscall(SYS_listen, fd, backlog);
}

int accept(int fd, struct sockaddr *addr, socklen_t *len)
{
    return syscall(SYS_accept, fd, addr, len);
}

int connect(int fd, struct sockaddr *addr, socklen_t len)
{
    return syscall(SYS_connect, fd, addr, len);
}

int shutdown(int fd, int how)
{
    return syscall(SYS_shutdown, fd, how);
}

int getsockname(int fd, struct sockaddr *addr, socklen_t *len)
{
    return syscall(SYS_getsockname, fd, addr, len);
}

int getpeername(int fd, struct sockaddr *addr, socklen_t *len)
{
    return syscall(SYS_getpeername, fd, addr, len);
}

ssize_t sendmsg(int fd, struct msghdr *msg, int flags)
{
    return syscall(SYS_sendmsg, fd, msg, flags);
}

ssize_t recvmsg(int fd, struct msghdr *msg, int flags)
{
    return syscall(SYS_recvmsg, fd, msg, flags);
}

ssize_t sendto(int fd, void *buf, size_t len, int flags, struct sockaddr *dst, socklen_t dlen)
{
    return syscall(SYS_sendto, fd, buf, len, flags, dst, dlen);
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src, socklen_t *slen)
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

void *mmap(void *addr, size_t len, int prot, int flgs, int fd, off_t off)
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

int getuid()
{
    return syscall(SYS_getuid);
}

int getgid()
{
    return syscall(SYS_getgid);
}

int geteuid()
{
    return syscall(SYS_geteuid);
}

int getegid()
{
    return syscall(SYS_getegid);
}

int setuid(uid_t uid)
{
    return syscall(SYS_setuid, uid);
}

int setgid(gid_t gid)
{
    return syscall(SYS_setgid, gid);
}

int setreuid(uid_t ruid, uid_t euid)
{
    return syscall(SYS_setreuid, ruid, euid);
}

int setregid(gid_t rgid, gid_t egid)
{
    return syscall(SYS_setregid, rgid, egid);
}

pid_t getpid()
{
    return syscall(SYS_getpid);
}

pid_t getppid()
{
    return syscall(SYS_getppid);
}
