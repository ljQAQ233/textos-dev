#ifndef	_UNISTD_H
#define	_UNISTD_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_size_t
#define __NEED_ssize_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_mode_t
#define __NEED_off_t
#define __NEED_pid_t
#define __NEED_intptr_t
#define __NEED_useconds_t

#include <bits/alltypes.h>

#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int pipe(int __fds[2]);
int close(int __fd);
off_t lseek(int __fd, off_t __off, int __whence);

int dup(int __fd);
int dup2(int __oldfd, int __newfd);

ssize_t read(int __fd, void *__buf, size_t __cnt);
ssize_t write(int __fd, const void *__buf, size_t __cnt);

int chown(char *__path, uid_t __owner, gid_t __group);
int fchown(int __fd, uid_t __owner, gid_t __group);

int mkdir(char *__path, mode_t __mode);
int rmdir(char *__path);
int chdir(char *__path);
int fchdir(int __fd); // TODO
char *getcwd(char *__buf, size_t __size);

pid_t fork();
int execve(char *__path, char *const __argv[], char *const __envp[]);
_Noreturn void _exit(int __stat);

int getuid();
int getgid();
int geteuid();
int getegid();
int setuid(uid_t __uid);
int setgid(gid_t __gid);
int setreuid(uid_t __ruid, uid_t __euid);
int setregid(gid_t __rgid, gid_t __egid);
int getgroups(int size, gid_t *list);
int setgroups(int size, gid_t *list); // non-posix

pid_t getpid();
pid_t getppid();

#if _XOPEN_CRYPT >= 0
char *crypt(const char *__key, const char *__salt);
#endif

__END_DECLS

#endif
