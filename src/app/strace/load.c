#include "strace.h"
#include <malloc.h>
#include <string.h>

#define MAX_ELEM_STR 32

int load_child_data(struct type *T, unsigned long addr, struct value *v)
{
    int count, nmemb;
    if (v->len == -1) {
        count = 1;
    } else {
        count = v->len;
    }
    if (T->cls == CLASS_BUF || T->cls == CLASS_STR) {
        nmemb = 1;
    } else if (T->cls == CLASS_ARR) {
        nmemb = T->ARR.elem_type->size;
    } else {
        nmemb = T->size;
    }

    if (T->cls == CLASS_STR) {
        long i;
        long longsiz = sizeof(long);
        char tmp[longsiz + 1];
        tmp[longsiz] = '\0';
        for (i = 0; i < MAX_ELEM_STR + 1; i += longsiz) {
            long rem = MAX_ELEM_STR - i;
            int chunk = rem < longsiz ? rem : longsiz;
            if (peek_data(pid, addr + i, tmp, chunk) < 0) return -1;
            int chunklen = strlen(tmp);
            if (chunklen != longsiz) {
                count = i + chunklen;
                break;
            }
        }
    }
    if (T->cls == CLASS_STR || T->cls == CLASS_BUF) {
        if (count > MAX_ELEM_STR) {
            count = MAX_ELEM_STR;
            v->is_truncated = 1;
        }
    }

    void *buf = malloc(count * nmemb);
    if (!buf) return -1;
    if (peek_data(pid, addr, buf, count * nmemb) < 0) return -1;

    v->ptr = buf;
    v->len = count;
    return 0;
}

int unload_child_data(struct type *T, struct value *v)
{
    free(v->ptr);
    return 0;
}

int load_data(struct type *T, unsigned long *field_ptr, struct value *v)
{
    v->ptr = 0;
    v->is_truncated = 0;
    if (!T->is_pointer) {
        if (T->cls == CLASS_STR || T->cls == CLASS_BUF) {
            size_t l = T->cls == CLASS_STR ? strlen(v->ptr) : v->len;
            if (l > MAX_ELEM_STR)
                v->len = MAX_ELEM_STR;
            else
                v->len = l;
        }
        v->ptr = field_ptr;
        return 0;
    }
    unsigned long addr = *field_ptr;
    return load_child_data(T, addr, v);
}

int unload_data(struct type *T, struct value *v)
{
    if (!T->is_pointer) return -1;
    return unload_child_data(T, v);
}
