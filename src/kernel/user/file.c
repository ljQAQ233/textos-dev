#include <textos/file.h>

ssize_t sys_write(int fd, void *buf, size_t cnt)
{
    return write(fd, buf, cnt);
}

ssize_t sys_read(int fd, void *buf, size_t cnt)
{
    return read(fd, buf, cnt);
}

int sys_close(int fd)
{
    return close(fd);
}
