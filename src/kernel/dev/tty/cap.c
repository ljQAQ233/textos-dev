#include <textos/dev/tty/cap.h>
#include <textos/dev/tty/tty.h>

#define CHECK(x, y)                                              \
    if (tio->x & ~y) {                                           \
        DEBUGK(K_DEBUG, "tio->" STR(x) " has unk bitmask %o\n"); \
        return false;                                            \
    }

bool tty_cap_check_attr(struct termios *tio)
{
    CHECK(c_iflag, TTY_CAP_IFLAG);
    CHECK(c_oflag, TTY_CAP_OFLAG);
    CHECK(c_cflag, TTY_CAP_CFLAG);
    CHECK(c_lflag, TTY_CAP_LFLAG);
    return true;
}
