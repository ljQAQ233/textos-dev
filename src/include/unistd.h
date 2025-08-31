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

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

off_t lseek(int __fd, off_t __off, int __whence);

int dup(int __fd);
int dup2(int __oldfd, int __newfd);

ssize_t read(int __fd, void *__buf, size_t __cnt);
ssize_t write(int __fd, const void *__buf, size_t __cnt);

#define F_OK 0
#define X_OK 0x01
#define W_OK 0x02
#define R_OK 0x04

int access(const char *__path, int __amode);

int chown(const char *__path, uid_t __owner, gid_t __group);
int fchown(int __fd, uid_t __owner, gid_t __group);

int mkdir(const char *__path, mode_t __mode);
int rmdir(const char *__path);
int chdir(const char *__path);
int fchdir(int __fd); // TODO
char *getcwd(char *__buf, size_t __size);

pid_t fork();
int execve(const char *__path, char *const __argv[], char *const __envp[]);
_Noreturn void _exit(int __stat);

/*
 * exec family suffix:
 *   * l - list
 *   * v - vector
 *   * e - environment
 *   * p - search in PATH
 */
int execl(const char *__path, const char *__arg, ...);
int execle(const char *__path, const char *__arg, ...);
int execlp(const char *__file, const char *__arg, ...);
int execv(const char *__path, char *const __argv[]);
int execvp(const char *__file, char *const __argv[]);
int execvpe(const char *__file, char *const __argv[], char *const __envp[]);

int getuid();
int getgid();
int geteuid();
int getegid();
int setuid(uid_t __uid);
int setgid(gid_t __gid);
int setreuid(uid_t __ruid, uid_t __euid);
int setregid(gid_t __rgid, gid_t __egid);
int getgroups(int __size, gid_t *__list);
int setgroups(int __size, gid_t *__list); // non-posix

pid_t getsid(pid_t __pid);
pid_t setsid();
pid_t getpgid(pid_t __pid);
int setpgid(pid_t __pid, pid_t __pgid);

pid_t getpid();
pid_t getppid();

#if _XOPEN_CRYPT >= 0
char *crypt(const char *__key, const char *__salt);
#endif

int gethostname(char *__name, size_t __len);
int sethostname(const char *__name, size_t __len);

#define _CS_PATH 0

size_t confstr(int name, char *buf, size_t size);

__END_DECLS

#endif
