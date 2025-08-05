#pragma once


#include "./internals.h"


typedef struct list_node {
    void *data;
    size_t type_size;

    struct list_node *next;
    struct list_node *prev;
} list_node;


typedef struct List {
    struct List *self;

    list_node *head;
    list_node *tail;
    size_t len;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    bool (*is_empty)(struct List *self);
    void (*print)(struct List *self);
    void (*append_at)(struct List *self, void *data, size_t type_size, size_t index);
    void (*modify_at)(struct List *self, void *data, size_t type_size, size_t index);
    void* (*get_at)(struct List *self, size_t index);
    void (*reverse)(struct List *self);
    void (*foreach)(struct List *self, void (*func)(void *data, va_list args), ...);
    void* (*delete_at)(struct List *self, size_t index);
    void (*push)(struct List *self, void *data, size_t type_size);
    void* (*pop)(struct List *self);
    void (*enqueue)(struct List *self, void *data, size_t type_size);
    void* (*dequeue)(struct List *self);
    bool (*lookup)(struct List *self, void *data, size_t type_size);
    size_t (*count_occurrences)(struct List *self, void *data, size_t type_size);
    void (*merge)(struct List *self, struct List *l2);
    void (*free)(struct List *self);

    void (*destroy)(void *data);
} List;


List* New_List(void (*destroy_func)(void *data));

