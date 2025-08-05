#pragma once


#include "./internals.h"


#define ARRAY_LIST_INITIAL_CAPACITY 128


typedef struct ArrayList {
    struct ArrayList *self;

    void **arr;
    size_t capacity;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    void (*set_at)(struct ArrayList *self, void *data, size_t type_size, size_t index);
    void* (*get_at)(struct ArrayList *self, size_t index);
    void (*foreach)(struct ArrayList *self, void (*func)(void *data, va_list args), ...);
    void (*free)(struct ArrayList *self);

    void (*destroy)(void *data);
} ArrayList;


ArrayList* New_ArrayList(void (*destroy_func)(void *data));

