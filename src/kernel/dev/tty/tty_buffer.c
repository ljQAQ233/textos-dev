#include <textos/dev/tty/tty_buffer.h>

void tty_buf_init(tty_buf_t *b)
{
    b->head = b->tail = 0;
}

int tty_buf_putc(tty_buf_t *b, char c)
{
    int next = (b->head + 1) % TTY_BUFSIZ;
    if (next == b->tail)
        return -1;
    b->buf[b->head] = c;
    b->flags[b->head] = 0;
    b->head = next;
    return 0;
}

/**
 * @brief get a character from buffer `b`
 * 
 * @param b tty buffer
 * @param c where to store?
 * @return int -1 is returned if no more space. or flag is returned on success
 */
int tty_buf_getc(tty_buf_t *b, char *c)
{
    u8 flag;
    if (b->head == b->tail)
        return -1;
    *c = b->buf[b->tail];
    flag = b->flags[b->tail];
    b->tail = (b->tail + 1) % TTY_BUFSIZ;
    return (int)flag;
}

int tty_buf_mark(tty_buf_t *b, u8 flag)
{
    if (b->head == b->tail)
        return -1;
    int pos = (b->head - 1 + TTY_BUFSIZ) % TTY_BUFSIZ;
    b->flags[pos] |= flag;
    return 0;
}

void tty_buf_kill(tty_buf_t *b)
{
    b->head = b->tail = 0;
}

int tty_buf_erase(tty_buf_t *b)
{
    if (b->head == b->tail)
        return -1;
    int prev = (b->head - 1 + TTY_BUFSIZ) % TTY_BUFSIZ;
    if (b->flags[prev] & TTY_BF_END)
        return 0;
    b->head = prev;
    return 0;
}

static inline int isspace(int c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

int tty_buf_erasew(tty_buf_t *b)
{
    if (b->head == b->tail)
        return 0;
    int count = 0;

    while (b->head != b->tail)
    {
        int prev = (b->head - 1 + TTY_BUFSIZ) % TTY_BUFSIZ;
        char c = b->buf[prev];
        u8 flag = b->flags[prev];
        if (flag & TTY_BF_END)
            break;
        if (!isspace(c))
            break;
        b->head = prev;
        count++;
    }

    while (b->head != b->tail)
    {
        int prev = (b->head - 1 + TTY_BUFSIZ) % TTY_BUFSIZ;
        char c = b->buf[prev];
        u8 flag = b->flags[prev];
        if (flag & TTY_BF_END)
            break;
        if (isspace(c))
            break;
        b->head = prev;
        count++;
    }

    return count;
}
