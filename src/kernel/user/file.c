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

RETVAL(ssize_t) sys_readdir(int fd, void *buf, size_t mx)
{
    return readdir(fd, buf, mx);
}

RETVAL(ssize_t) sys_read(int fd, void *buf, size_t cnt)
{
    return read(fd, buf, cnt);
}

RETVAL(int) sys_close(int fd)
{
    return close(fd);
}

RETVAL(int) sys_stat(char *path, stat_t *sb)
{
    return stat(path, sb);
}

RETVAL(int) sys_ioctl(int fd, int req, void *argp)
{
    return ioctl(fd, req, argp);
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

RETVAL(int) sys_mknod(char *path, int mode, long dev)
{
    return mknod(path, mode, dev);
}

RETVAL(int) sys_mount(char *src, char *dst)
{
    return mount(src, dst);
}

RETVAL(int) sys_umount2(char *target, int flags)
{
    return umount2(target, flags);
}

RETVAL(int) sys_chdir(char *path)
{
    return chdir(path);
}

RETVAL(int) sys_mkdir(char *path, int mode)
{
    return mkdir(path, mode);
}

RETVAL(int) sys_rmdir(char *path)
{
    return rmdir(path);
}