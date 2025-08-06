#pragma once


#include "./internals.h"


#define ARRAY_LIST_INITIAL_CAPACITY 128


typedef struct {
    void *data;
    size_t type_size;
} ArrayListElement;


typedef struct ArrayList {
    // the self pointer is optional to use for the user
    struct ArrayList *self;

    ArrayListElement **arr;
    size_t capacity;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    // given self, the data, the type_size and an index
    // stores the data at index
    void (*set_at)(struct ArrayList *self, void *data, size_t type_size, size_t index);

    // retuns a copy of the data stored at (index)
    void* (*get_at)(struct ArrayList *self, size_t index);

    // applies (func) to every data
    void (*foreach)(struct ArrayList *self, void (*func)(void *data, va_list args), ...);
    
    // free the ArrayList
    void (*free)(struct ArrayList *self);

    // custom destroyer which is applied to every data
    void (*destroy)(void *data);
} ArrayList;


ArrayList* New_ArrayList(void (*destroy_func)(void *data));

