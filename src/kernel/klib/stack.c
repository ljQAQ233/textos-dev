#include <textos/klib/stack.h>
#include <textos/mm.h>

typedef stacki_t elem_t;

stack_t *stack_init (stack_t *stk)
{
    if (!stk) {
        stk = malloc(sizeof(stack_t));
        stk->fixed = false;
    } else {
        stk->fixed = true;
    }

    stk->siz = 0;
    stk->top = NULL;

    stk->pop = NULL;
    stk->clr = NULL;

    return stk;
}

void stack_set (stack_t *stk, void *clr, void *pop)
{
    stk->pop = pop;
    stk->clr = clr;
}

void stack_fini (stack_t *stk)
{
    stack_clear (stk);
    if (!stk->fixed)
        free(stk);
}

void stack_push (stack_t *stk, void *payload)
{
    elem_t *elem = malloc(sizeof(elem_t));
    elem->payload = payload;
    elem->next = stk->top;

    // Replace the top
    stk->top = elem;
    stk->siz++;
}

void *stack_top (stack_t *stk)
{
    if (stack_empty(stk))
        return NULL;

    return ((elem_t *)stk->top)->payload;
}

void stack_pop (stack_t *stk)
{
    if (stack_empty(stk))
        return;

    elem_t *Top = stk->top;
    stk->top = Top->next;

    if (stk->pop)
        stk->pop (Top);
    stk->siz--;
}

void stack_clear (stack_t *stk)
{
    while (!stack_empty (stk))
    {
        if (stk->clr)
            stk->clr (stk->top);
        stack_pop (stk);
    }

    stk->siz = 0;
}

bool stack_empty (stack_t *stk)
{
    return !stk->top;
}

size_t stack_siz (stack_t *stk)
{
    return stk->siz;
}

