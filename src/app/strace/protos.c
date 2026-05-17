#include "strace.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

#define defproto(N)   struct proto _proto_##N[] = {
#define p_entry(N, T) {N, refty_type(T)}
#define endproto() \
    {              \
        NULL       \
    }              \
    }

#define refproto(N) (_proto_##N)

#define regfun(N)                        \
    struct type _fun_##N = {             \
        .name = _STR(N),                 \
        .size = 0,                       \
        .cls = CLASS_PROTO,              \
        .printer = proto_printer,        \
        .PROTO = {.proto = refproto(N)}, \
    }

#define entry(t, a) p_entry(_STR(a), t),

#define MAP0(m, ...)
#define MAP1(m, t, a, ...) m(t, a)
#define MAP2(m, t, a, ...) m(t, a) MAP1(m, __VA_ARGS__)
#define MAP3(m, t, a, ...) m(t, a) MAP2(m, __VA_ARGS__)
#define MAP4(m, t, a, ...) m(t, a) MAP3(m, __VA_ARGS__)
#define MAP5(m, t, a, ...) m(t, a) MAP4(m, __VA_ARGS__)
#define MAP6(m, t, a, ...) m(t, a) MAP5(m, __VA_ARGS__)
#define MAP(n, ...)        _CONCAT(MAP, n)(__VA_ARGS__)

// 数数有几个参数呢, type arg 算一对哦.
// unused 防止没有参数反而还有一个逗号的情况, 由于这里加入了一个
// unused, _CNT 中就要偏移回来. e 是为了防止奇数个的 宏参数
#define CNT(...)  _CNT(unused, ##__VA_ARGS__)
#define _CNT(...) __CNT(__VA_ARGS__, e, 6, e, 5, e, 4, e, 3, e, 2, e, 1, e, 0)

#define __CNT(t0, a0, t1, a1, t2, a2, t3, a3, t4, a4, t5, a5, t6, a6, N, ...) N

#define autoproto(ret, func, ...)                         \
    struct proto _proto_##func[] = {                      \
        p_entry("retval", ret),                           \
        MAP(CNT(__VA_ARGS__), entry, __VA_ARGS__){NULL}}; \
    regfun(func)

// 占个位置先
#define noneproto(ret, func, ...)                                          \
    struct proto _proto_##func[] = {                                       \
        p_entry("retval", long), p_entry("a1", long), p_entry("a2", long), \
        p_entry("a3", long),     p_entry("a4", long), p_entry("a5", long), \
        p_entry("a6", long)};                                              \
    regfun(func)

autoproto(char *, getcwd, char *, buf, size_t, size);
autoproto(clock_t, times, struct tms *, buf);
autoproto(int, __seekdir, int, fd, size_t *, pos);
autoproto(int, accept, int, fd, struct sockaddr *, addr, socklen_t *, len);
autoproto(int, access, char *, path, int, amode);
autoproto(int, bind, int, fd, struct sockaddr *, addr, socklen_t, len);
autoproto(int, chdir, char *, path);
autoproto(int, chmod, char *, path, mode_t, mode);
autoproto(int, chown, char *, path, uid_t, owner, gid_t, group);
autoproto(int, close, int, fd);
autoproto(int, connect, int, fd, struct sockaddr *, addr, socklen_t, len);
autoproto(int, dup, int, fd);
autoproto(int, dup2, int, oldfd, int, newfd);
autoproto(int, execve, char *, path, char **, char **envp);
autoproto(int, fchmod, int, fd, mode_t, mode);
autoproto(int, fchown, int, fd, uid_t, owner, gid_t, group);
noneproto(int, fcntl, int, fd, int, cmd, ...);
autoproto(int, fork);
autoproto(int, fstat, int, fd, struct stat *, sb);
autoproto(int, getegid);
autoproto(int, geteuid);
autoproto(int, getgid);
autoproto(int, getgroups, int, size, gid_t *, list);
autoproto(int, gethostname, char *, name, size_t, len);
autoproto(int, getpeername, int, fd, struct sockaddr *, addr, socklen_t *, len);
autoproto(int, getsockname, int, fd, struct sockaddr *, addr, socklen_t *, len);
autoproto(int, gettimeofday, struct timeval *, tp, void *, tzp);
autoproto(int, getuid);
noneproto(int, ioctl, int, fd, int, req, ...);
autoproto(int, kill, int, pid, int, sig);
autoproto(int, listen, int, fd, int, backlog);
autoproto(int, lstat, char *, path, struct stat *, sb);
autoproto(int, mkdir, char *, path, mode_t, mode);
autoproto(int, mknod, char *, path, mode_t, mode, dev_t, dev);
autoproto(int, mount, char *, src, char *, dst);
autoproto(int, mprotect, void *, addr, size_t, len, int, prot);
autoproto(int, munmap, void *, addr, size_t, len);
autoproto(int, nanosleep, struct timespec *, rqtp, struct timespec *, rmtp);
noneproto(int, open, char *, path, int, flgs, ...);
autoproto(int, pause);
noneproto(int, pipe, int fds[2]);
autoproto(int, raise, int, sig);
autoproto(int, rmdir, char *, path);
autoproto(int, setgid, gid_t, gid);
autoproto(int, setgroups, int, size, gid_t *, list);
autoproto(int, sethostname, char *, name, size_t, len);
autoproto(int, setpgid, pid_t, pid, pid_t, pgid);
autoproto(int, setregid, gid_t, rgid, gid_t, egid);
autoproto(int, setreuid, uid_t, ruid, uid_t, euid);
autoproto(int, setuid, uid_t, uid);
autoproto(int, shutdown, int, fd, int, how);
autoproto(int, sigaction, int, signum, struct sigaction *, act,
          struct sigaction *, oldact);
autoproto(int, sigprocmask, int, how, sigset_t *, set, sigset_t *, oset);
autoproto(int, socket, int, domain, int, type, int, proto);
autoproto(int, stat, char *, path, struct stat *, sb);
autoproto(int, umount, char *, target);
autoproto(int, umount2, char *, target, int, flags);
autoproto(int, uname, struct utsname *, name);
autoproto(int, wait, int *, stat);
autoproto(int, wait4, int, pid, int *, stat, int, opt, void *, rusage);
autoproto(off_t, lseek, int, fd, off_t, off, int, whence);
autoproto(pid_t, getpgid, pid_t, pid);
autoproto(pid_t, getpid);
autoproto(pid_t, getppid);
autoproto(pid_t, getsid, pid_t, pid);
autoproto(pid_t, setsid);
autoproto(ssize_t, __readdir, int, fd, void *, buf, size_t, mx);
autoproto(ssize_t, read, int, fd, void *, buf, size_t, cnt);
autoproto(ssize_t, readv, int, fd, struct iovec *, iov, int, iovcnt);
autoproto(ssize_t, recv, int, fd, void *, buf, size_t, len, int, flags);
autoproto(ssize_t, recvfrom, int, fd, void *, buf, size_t, len, int, flags,
          struct sockaddr *, src, socklen_t *, slen);
autoproto(ssize_t, recvmsg, int, fd, struct msghdr *, msg, int, flags);
autoproto(ssize_t, send, int, fd, void *, buf, size_t, len, int, flags);
autoproto(ssize_t, sendmsg, int, fd, struct msghdr *, msg, int, flags);
autoproto(ssize_t, sendto, int, fd, void *, buf, size_t, len, int, flags,
          struct sockaddr *, dst, socklen_t, dlen);
autoproto(ssize_t, write, int, fd, void *, buf, size_t, cnt);
autoproto(ssize_t, writev, int, fd, struct iovec *, iov, int, iovcnt);
autoproto(time_t, time, time_t *, tp);
autoproto(void *, signal, int, signum, void *, handler);
autoproto(void *, brk, void *, ptr);
autoproto(void *, mmap, void *, addr, size_t, len, int, prot, int, flgs, int,
          fd, off_t, off);

//
// special functions
//
noneproto(void, sigreturn);       // __restorer
noneproto(void, exit, int, stat); // exit syscall
