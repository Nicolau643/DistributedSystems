#ifndef _LIST_PRIVATE_H
#define _LIST_PRIVATE_H

#include "list.h"
#include "entry.h"

struct node_t {
    struct entry_t* entry;
    struct node_t* next;
};

struct list_t {
    int size;
    struct node_t* first;
    struct node_t* last;
};

struct node_t *node_create(struct entry_t* entry);

void node_destroy(struct node_t* node);

void list_print(struct list_t* list);

#endif
