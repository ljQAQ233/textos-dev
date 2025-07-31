#pragma once

#include <termios.h>
#include <textos/ktimer.h>
#include <textos/dev/tty/stream.h>
#include <textos/dev/tty/tty_buffer.h>

typedef struct
{
    int stop;
    int istop;
    pid_t pgrp;
    pid_t iwaiter;
    pid_t owaiter;
    tty_buf_t ibuf;
    tty_buf_t obuf;
    size_t reqlen;
    bool timeout;
    ktimer_t timer;
    struct termios tio;
    tty_ios_t ios;
} tty_t;
