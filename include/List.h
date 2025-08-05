#pragma once


#include "./internals.h"


typedef struct list_node {
    void *data;
    size_t type_size;

    struct list_node *next;
    struct list_node *prev;
} list_node;


typedef struct List {
    // the self pointer is optional to use for the user
    struct List *self;

    list_node *head;
    list_node *tail;

    size_t len;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    // returns self->len==0
    bool (*is_empty)(struct List *self);

    // prints the List in stdout
    void (*print)(struct List *self);

    // given the list, the data, the size of the (*data) and an index
    // appends a new_node in the given index.
    // throws and error if index > self->len
    void (*append_at)(struct List *self, void *data, size_t type_size, size_t index);

    // given an index, updates the data of the node at the (index) index
    // throws and error if index >= self->len
    void (*modify_at)(struct List *self, void *data, size_t type_size, size_t index);

    // returns the data pointed by the (index) node
    // throws an error if index >= self->len
    void* (*get_at)(struct List *self, size_t index);

    // reverse the list
    void (*reverse)(struct List *self);

    // applies (func) to every node's data
    void (*foreach)(struct List *self, void (*func)(void *data, va_list args), ...);

    // given an index, deletes the (index) node
    // throws an error if index >= self->len
    void* (*delete_at)(struct List *self, size_t index);

    // push a new node into the list
    void (*push)(struct List *self, void *data, size_t type_size);

    // pop the list's last node and 
    // returns a copy with copy_from_void_ptr (internals.h)
    void* (*pop)(struct List *self);

    // enqueue a new node into the list
    void (*enqueue)(struct List *self, void *data, size_t type_size);
    
    // dequeue 
    // returns a copy with copy_from_void_ptr (internals.h)
    void* (*dequeue)(struct List *self);

    // returns if any node's points to an equal chuck of memory as data
    bool (*lookup)(struct List *self, void *data, size_t type_size);

    // counts the number of nodes that points to an equal chuck of memory as data
    size_t (*count_occurrences)(struct List *self, void *data, size_t type_size);

    // merge two lists into self
    void (*merge)(struct List *self, struct List *l2);

    // free the list
    void (*free)(struct List *self);

    // custom destroyer which is applied to every node's data ptr
    void (*destroy)(void *data);
} List;


List* New_List(void (*destroy_func)(void *data));

