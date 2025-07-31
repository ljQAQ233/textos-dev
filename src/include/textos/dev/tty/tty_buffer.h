#pragma once

#define TTY_BUFSIZ 4096
#define TTY_XONSIZ 0.8 * TTY_BUFSIZ

// elements' flag
#define TTY_BF_END (1 << 0)

typedef struct
{
    char buf[TTY_BUFSIZ];
    u8 flags[TTY_BUFSIZ];
    int head;
    int tail;
} tty_buf_t;

void tty_buf_init(tty_buf_t *b);
int tty_buf_full(tty_buf_t *b);
int tty_buf_chrs(tty_buf_t *b);
int tty_buf_rems(tty_buf_t *b);
int tty_buf_putc(tty_buf_t *b, char c);
int tty_buf_getc(tty_buf_t *b, char *c);
int tty_buf_mark(tty_buf_t *b, u8 flag);
void tty_buf_kill(tty_buf_t *b);
int tty_buf_erase(tty_buf_t *b);
int tty_buf_erasew(tty_buf_t *b);
