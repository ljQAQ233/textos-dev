#pragma once

// fifo (char)
typedef struct
{
    char *buf;
    size_t siz;
    size_t head;
    size_t tail;
} fifo_t;

void fifo_clear(fifo_t *fifo);

bool fifo_full(fifo_t *fifo);

fifo_t *fifo_init(fifo_t *fifo, char *buf, size_t siz);

size_t fifo_read(fifo_t *fifo, char *buf, size_t siz);

size_t fifo_write(fifo_t *fifo, char *data, size_t siz);
