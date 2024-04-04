/*
 * double linked list
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

#include <textos/args.h>
#include <textos/klib/list.h>

void list_init(list_t *list)
{
    list->prev = list;
    list->next = list;
}

void list_insert_after(list_t *list, list_t *node)
{
    node->next = list->next;
    node->next->prev = node;
    node->prev = list;
    list->next = node;
}

void list_insert_before(list_t *list, list_t *node)
{
    list_insert_after(list->prev, node);
}

void list_insert(list_t *list, list_t *node)
{
    list_insert_after(list, node);
}

void list_remove(list_t *list)
{
    if (list->prev == list)
        return;

    list->prev->next = list->next;
    list->next->prev = list->prev;
}

bool list_empty(list_t *list)
{
    return list == list->prev ? true : false;
}

