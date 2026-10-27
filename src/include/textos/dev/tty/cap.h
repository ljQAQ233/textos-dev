#pragma once

struct termios;

#define TTY_CAP_IFLAG (ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF)
#define TTY_CAP_OFLAG (OPOST | OLCUC | ONLCR)
#define TTY_CAP_CFLAG (0)
#define TTY_CAP_LFLAG (ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL | NOFLSH)

bool tty_cap_check_attr(struct termios *tio);
