#include "stdio.h"

static FILE *ofl_head;

void __ofl_add(FILE *f)
{
    f->next = ofl_head;
    ofl_head = f;
}

void __ofl_del(FILE *f)
{
    FILE **pp = &ofl_head;
    while (*pp && *pp != f)
        pp = (FILE **)&((*pp)->next);
    if (*pp == f)
        *pp = (*pp)->next;
}

FILE *__ofl_get()
{
    return ofl_head;
}