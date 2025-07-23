#ifndef _TERMIOS_H
#define _TERMIOS_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_pid_t
#define __NEED_struct_winsize

#include <bits/alltypes.h>

typedef unsigned char cc_t;
typedef unsigned int speed_t;
typedef unsigned int tcflag_t;

#define NCCS 32

struct termios
{
    tcflag_t c_iflag;
    tcflag_t c_oflag;
    tcflag_t c_cflag;
    tcflag_t c_lflag;
    cc_t c_line;
    cc_t c_cc[NCCS];
    speed_t __c_ispeed;
    speed_t __c_ospeed;
};

// input flags          posix ?
#define IGNCR   0000200 // Y Ignore CR on input.
#define ICRNL   0000400 // Y Translate carriage return to newline on input.
#define IXON    0002000 // Y Enable XON/XOFF flow control on output.
#define IXOFF   0010000 // Y Enable XON/XOFF flow control on input. TODO

// output flags        posix ?
#define OPOST  0000001 // Y Enable output processing
#define OLCUC  0000002 // N Map lowercase letters to uppercase on output
#define ONLCR  0000004 // Y Map newline '\n' to carriage return + newline "\r\n"

// control flags       posix ?
#define CSIZE  0000060 // Y Character size mask.
#define CS5    0000000 // Y
#define CS6    0000020 // Y
#define CS7    0000040 // Y
#define CS8    0000060 // Y 
#define CREAD  0000200 // Y
#define CLOCAL 0004000 // Y Ignore modem control lines. TODO

// local flags         posix ?
#define ISIG   0000001 // Y Generate the corresponding signal for INTR, QUIT, SUSP, or DSUSP.
#define ICANON 0000002 // Y Enable canonical mode
#define ECHO   0000010 // Y Echo input characters.
#define ECHOE  0000020 // Y
#define ECHOK  0000040 // Y
#define ECHONL 0000100 // Y
#define NOFLSH 0000200 // Y
#define TOSTOP 0000400 // Y
#define IEXTEN 0100000 // N Enable implementation-defined input processing

// c_cc flags
#define VINTR     0
#define VQUIT     1
#define VERASE    2
#define VKILL     3
#define VEOF      4
#define VTIME     5
#define VMIN      6
#define VSWTC     7
#define VSTART    8
#define VSTOP     9
#define VSUSP    10
#define VEOL     11
#define VREPRINT 12
#define VDISCARD 13
#define VWERASE  14
#define VLNEXT   15
#define VEOL2    16

// optional actions
#define TCSANOW   0
#define TCSADRAIN 1
#define TCSAFLUSH 2

int tcgetattr(int __fd, struct termios *__tio);
int tcsetattr(int __fd, int __act, const struct termios *__tio);

__END_DECLS

#endif
