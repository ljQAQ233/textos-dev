#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>

int tcgetattr(int fd, struct termios *tio)
{
    return ioctl(fd, TCGETS, tio);
}

int tcsetattr(int fd, int act, const struct termios *tio)
{
    if (act < 0 || act > 2)
    {
        errno = EINVAL;
        return -1;
    }
    return ioctl(fd, TCSETS + act, tio);
}
