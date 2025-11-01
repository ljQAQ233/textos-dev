#pragma once

#include <termios.h>
#include <textos/task.h>
#include <textos/ktimer.h>
#include <textos/dev/tty/tty_buffer.h>

typedef int (*tty_ioctl_t)(void *io, int req, void *argp);
typedef ssize_t (*tty_iosop_t)(void *io, char *s, size_t len);

typedef struct
{
    int stop;
    int istop;
    pid_t pgrp;
    task_t *iwaiter;
    task_t *owaiter;
    tty_buf_t ibuf;
    tty_buf_t obuf;
    size_t reqlen;
    bool timeout;
    ktimer_t timer;
    struct termios tio;
    
    void *data;
    tty_ioctl_t ctl;
    tty_iosop_t in;
    tty_iosop_t out;
} tty_t;

tty_t *tty_register(tty_t *tty, char *name, void *data, tty_ioctl_t ctl, tty_iosop_t in, tty_iosop_t out);
