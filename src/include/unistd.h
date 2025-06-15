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

int pipe(int fds[2]);
int close(int fd);

int dup(int fd);
int dup2(int oldfd, int newfd);

ssize_t read(int fd, void *buf, size_t cnt);
ssize_t write(int fd, const void *buf, size_t cnt);

int mkdir(char *path, mode_t mode);
int rmdir(char *path);
int chdir(char *path);
int fchdir(int fd); // TODO
char *getcwd(char *buf, size_t size); // TODO

pid_t fork();
int execve(char *path, char *const argv[], char *const envp[]);
_Noreturn void _exit(int stat);

pid_t getpid();
pid_t getppid();

__END_DECLS

#endif
