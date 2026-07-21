#include <malloc.h>

struct kv
{
    int d, k;
    void *v;
    struct kv *next;
};

static struct kv *next;

void *__w_setval(int d, int k, void *v)
{
    struct kv *kv = malloc(sizeof(struct kv));
    kv->d = d;
    kv->k = k;
    kv->v = v;
    kv->next = next;
    next = kv;
    return v;
}

void *__w_getval(int d, int k)
{
    for (struct kv *c = next; c; c = c->next)
        if (c->d == d && c->k == k) return c->v;
    return 0;
}

void __w_delval(int d, int k)
{
    struct kv *c = next;
    struct kv **pp = &next;
    for (; c; c = c->next) {
        if (c->d == d && c->k == k) {
            *pp = c->next;
            free(c);
        }
        pp = &c->next;
    }
}
