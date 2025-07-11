// 这样粗暴的 "堆" 直接集成在 可执行文件中 了 (.bss)

#include <errno.h>
#include <stdint.h>
#include <malloc.h>

#define MAXN (1 << 16)

uint8_t heap[MAXN];

void *ap = heap;

void *malloc(size_t siz)
{
    void *ptr = NULL;
    if (ap + siz < (void *)heap + MAXN) {
        ptr = ap;
        ap += siz;
    } else
        errno = ENOMEM;
    return ptr;
}

void free(void *ptr)
{
    // nothing to do
}