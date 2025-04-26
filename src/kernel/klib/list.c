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

void list_pushhead(list_t *list, list_t *node)
{
    list_insert_after(list, node);
}

void list_pushback(list_t *list, list_t *node)
{
    list_insert_before(list, node);
}

list_t *list_pophead(list_t *list)
{
    list_t *head = list->next;
    list_remove(head);
    return head;
}

list_t *list_popback(list_t *list)
{
    list_t *back = list->prev;
    list_remove(back);
    return back;
}

bool list_empty(list_t *list)
{
    return list == list->prev ? true : false;
}

