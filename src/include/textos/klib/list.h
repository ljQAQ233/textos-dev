/*
 * double-linked list
 * macro CR makes it useful - to get the record by a member in it
 *
 * In fact a few projects use forward / backward but it leads to confusion
 * so next / prev is better to understand, here's the structure.
 *
 *  list 0              list 1             list x
 *  +------+------+     +------+------+
 *  | prev | next | <-> | prev | next | <-> ... <-> list 0
 *  +------+------+     +------+------+
 */
#pragma once

typedef struct list
{
    struct list *prev;
    struct list *next;
} list_t;

static inline void list_init(list_t *list)
{
    list->prev = list;
    list->next = list;
}

static inline void list_insert_after(list_t *list, list_t *node)
{
    node->next = list->next;
    node->next->prev = node;
    node->prev = list;
    list->next = node;
}

static inline void list_insert_before(list_t *list, list_t *node)
{
    list_insert_after(list->prev, node);
}

static inline void list_insert(list_t *list, list_t *node)
{
    list_insert_after(list, node);
}

static inline void list_remove(list_t *list)
{
    if (list->prev == list) return;
    list->prev->next = list->next;
    list->next->prev = list->prev;
}

static inline void list_pushhead(list_t *list, list_t *node)
{
    list_insert_after(list, node);
}

static inline void list_pushback(list_t *list, list_t *node)
{
    list_insert_before(list, node);
}

static inline list_t *list_pophead(list_t *list)
{
    list_t *head = list->next;
    list_remove(head);
    return head;
}

static inline list_t *list_popback(list_t *list)
{
    list_t *back = list->prev;
    list_remove(back);
    return back;
}

static inline bool list_empty(list_t *list)
{
    return list == list->prev;
}

#define LIST_INIT(list)          \
    ((list_t){                   \
        .prev = (list_t *)&list, \
        .next = (list_t *)&list, \
    })

#define LIST_FOREACH(iter, list) \
    for (iter = (list)->next; iter != (list); iter = iter->next)

#define LIST_FOREACH_REV(iter, list) \
    for (iter = (list)->prev; iter != (list); iter = iter->prev)
