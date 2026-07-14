#include "stdio.h"

static char __stdin_buf[BUFSIZ + MAX_UNGETC];
static char __stdout_buf[BUFSIZ + MAX_UNGETC];

FILE __stdio_stdin = {
    .fd = 0,
    .fl = F_PERM | F_NOWR,
    .lbf = '\n',
    .pos = 0,
    .bufsz = BUFSIZ,
    .buf = __stdin_buf - MAX_UNGETC,
    .read = __stdio_read,
    .write = __stdio_write,
    .close = __stdio_close,
};

FILE __stdio_stdout = {
    .fd = 1,
    .fl = F_PERM | F_NORD,
    .lbf = '\n',
    .pos = 0,
    .bufsz = BUFSIZ - MAX_UNGETC,
    .buf = __stdout_buf,
    .read = __stdio_read,
    .write = __stdio_write,
    .close = __stdio_close,
};

FILE __stdio_stderr = {
    .fd = 2,
    .fl = F_PERM | F_NORD,
    .lbf = EOF,
    .pos = 0,
    .bufsz = 0,
    .buf = NULL,
    .read = __stdio_read,
    .write = __stdio_write,
    .close = __stdio_close,
};

void __init_stdio()
{
    __ofl_add(stdin);
    __ofl_add(stdout);
    __ofl_add(stderr);
}
    
void __fini_stdio()
{
    FILE *c = __ofl_get(), *n;
    while (c) {
        n = c->next;
        __fclosex(c);
        c = n;
    }
}
