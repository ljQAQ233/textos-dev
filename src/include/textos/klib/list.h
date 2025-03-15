#pragma once

struct list;
typedef struct list list_t;

struct list {
    list_t *prev;
    list_t *next;
};

void list_init(list_t *list);

void list_insert_after(list_t *list, list_t *node);

void list_insert_before(list_t *list, list_t *node);

void list_insert(list_t *list, list_t *node);

void list_remove(list_t *list);

void list_push(list_t *list, list_t *node);

list_t *list_pop(list_t *list);

bool list_empty (list_t *list);

#define LIST_INIT(list)           \
  ((list_t){                      \
      .prev = (list_t *)&list,    \
      .next = (list_t *)&list,    \
  })

#define LIST_FOREACH(iter, list)  \
    for (iter = (list)->next ;    \
         iter != (list) ;         \
         iter = iter->next        \
    )

#define LIST_FOREACH_REV(iter, list)  \
    for (iter = (list)->prev ;    \
         iter != (list) ;         \
         iter = iter->prev        \
    )
