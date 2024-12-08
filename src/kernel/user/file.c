#include <textos/file.h>
#include <textos/syscall.h>

RETVAL(int) sys_open(char *path, int flgs)
{
    return open(path, flgs);
}

RETVAL(ssize_t) sys_write(int fd, void *buf, size_t cnt)
{
    return write(fd, buf, cnt);
}

RETVAL(ssize_t) sys_read(int fd, void *buf, size_t cnt)
{
    return read(fd, buf, cnt);
}

RETVAL(int) sys_close(int fd)
{
    return close(fd);
}

RETVAL(int) sys_dup(int fd)
{
    return dup(fd);
}

RETVAL(int) sys_dup2(int old, int new)
{
    return dup2(old, new);
}

RETVAL(int) sys_pipe(int fds[2])
{
    return pipe(fds);
}
