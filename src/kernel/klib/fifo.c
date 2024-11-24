#include <textos/assert.h>
#include <textos/klib/fifo.h>

#define HEAD (fifo->head % fifo->siz)
#define TAIL (fifo->tail % fifo->siz)

fifo_t *fifo_init(fifo_t *fifo, char *buf, size_t siz)
{
    ASSERTK (siz != 0);

    if (!buf)
        return NULL;
    
    if (!fifo)
        return NULL;
    
    fifo->buf = buf;
    fifo->siz = siz;
    fifo_clear(fifo);

    return fifo;
}

size_t fifo_write(fifo_t *fifo, char *data, size_t siz)
{
    char *p = data;
    size_t ret = 0,
           rem = MIN(siz, fifo->siz - (fifo->tail - fifo->siz - fifo->head));
    while (rem != 0) {
        fifo->buf[TAIL] = *p;
        fifo->tail++;
        rem--, ret++, p++;
    }
    return ret;
}

size_t fifo_read(fifo_t *fifo, char *buf, size_t siz)
{
    char *p = buf;
    size_t ret = 0,
           rem = MIN(siz, fifo->tail - fifo->siz - fifo->head);
    while (rem != 0) {
        *p = fifo->buf[HEAD];
        fifo->head++;
        rem--, ret++, p++;
    }
    return ret;
}

bool fifo_full(fifo_t *fifo)
{
    return fifo->head == fifo->tail;
}

bool fifo_empty(fifo_t *fifo)
{
    return fifo->head + fifo->siz == fifo->tail;
}

void fifo_clear(fifo_t *fifo)
{
    fifo->head = 0;
    fifo->tail = fifo->siz;
}