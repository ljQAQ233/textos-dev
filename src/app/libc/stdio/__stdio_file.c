#include "stdio.h"

static char __stdin_buf[BUFSIZ];
static char __stdout_buf[BUFSIZ];
static char __stderr_buf[BUFSIZ];

FILE __stdio_stdin = {
    .fd = 0,
    .fl = F_PERM | F_NOWR,
    .lbf = '\n',
    .pos = 0,
    .bufsz = BUFSIZ,
    .buf = __stdin_buf,
    .read = __stdio_read,
    .write = __stdio_write,
    .close = __stdio_close,
};

FILE __stdio_stdout = {
    .fd = 1,
    .fl = F_PERM | F_NORD,
    .lbf = '\n',
    .pos = 0,
    .bufsz = BUFSIZ,
    .buf = __stdout_buf,
    .read = __stdio_read,
    .write = __stdio_write,
    .close = __stdio_close,
};

FILE __stdio_stderr = {
    .fd = 2,
    .fl = F_PERM | F_NORD,
    .lbf = '\n',
    .pos = 0,
    .bufsz = 0,
    .buf = NULL,
    .read = __stdio_read,
    .write = __stdio_write,
    .close = __stdio_close,
};

__attribute__((constructor))
static void __init_stdio()
{
    __ofl_add(stdin);
    __ofl_add(stdout);
    __ofl_add(stderr);
}
