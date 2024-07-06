#pragma once

typedef void (__stack_pop)(void *payload);
typedef void (__stack_clr)(void *payload);

typedef struct
{
    size_t      siz;
    void        *top;
    bool        fixed;
    __stack_pop *pop;
    __stack_clr *clr;
} stack_t;

stack_t *stack_init (stack_t *stk);

void stack_set (stack_t *stk, void *clr, void *pop);

void stack_fini (stack_t *stk);

void stack_push (stack_t *stk, void *payload);

void *stack_top (stack_t *stk);

void stack_pop (stack_t *stk);

void stack_clear (stack_t *stk);

bool stack_empty (stack_t *stk);

/* return the number of elements in this `stk` */
size_t stack_siz (stack_t *stk);

typedef struct stacki
{
    void          *payload;
    struct stacki *next;
} stacki_t;

//

#define stacki(stk, iter) \
        (iter = ((stk)->top))

#define stacki_next(iter) \
        (iter = iter ? ((stacki_t *)iter)->next)

#define stacki_data(iter) \
        ((stacki_t *)iter->payload)

