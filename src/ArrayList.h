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
} ArrayList;


ArrayList* New_ArrayList();
static inline void ArrayList_set_at(ArrayList *self, void *data, size_t type_size, size_t index);
static inline void* ArrayList_get_at(ArrayList *self, size_t index);
static void ArrayList_foreach(ArrayList *self, void (*func)(void *data, va_list args), ...);
static inline void ArrayList_free(ArrayList *self);


ArrayList* New_ArrayList() {
    ArrayList *self = (ArrayList*) Malloc(sizeof(ArrayList));

    pthread_mutexattr_init(&self->mutex_attr);
    pthread_mutexattr_settype(&self->mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&self->mutex, &self->mutex_attr);

    pthread_mutexattr_destroy(&self->mutex_attr);

    self->capacity = ARRAY_LIST_INITIAL_CAPACITY;
    self->arr = (void**) Calloc(self->capacity, sizeof(void*));
    
    self->set_at = ArrayList_set_at;
    self->get_at = ArrayList_get_at;
    self->foreach = ArrayList_foreach;
    self->free = ArrayList_free;

    return self;
}


static inline void ArrayList_set_at(ArrayList *self, void *data, size_t type_size, size_t index) {
    LOCK(self->mutex);

    if (index >= self->capacity) {
        size_t new_capacity = self->capacity * 2;

        while (index >= new_capacity)
            new_capacity *= 2;

        void **new_arr = (void**) Realloc(self->arr, new_capacity * sizeof(void*));

        memset(new_arr + self->capacity, 0, (new_capacity - self->capacity) * sizeof(void*));

        self->arr = new_arr;
        self->capacity = new_capacity;
    }

    if (data) {
        free(self->arr[index]);
        self->arr[index] = copy_from_void_ptr(data, type_size);
    } else
        self->arr[index] = NULL;

    UNLOCK(self->mutex);
}


static inline void* ArrayList_get_at(ArrayList *self, size_t index) {
    LOCK(self->mutex);

    void *val = NULL;

    if (index < self->capacity) 
        val = self->arr[index];

    UNLOCK(self->mutex);

    return val;
}


static void ArrayList_foreach(ArrayList *self, void (*func)(void *data, va_list args), ...) {
    LOCK(self->mutex);

    va_list args;
    va_start(args, func);

    for (size_t i=0; i<self->capacity; ++i) {
        void *data = self->get_at(self, i);
        
        if (!data)
            continue;
        
        va_list args_copy;
        va_copy(args_copy, args);
        func(data, args_copy);
        va_end(args_copy);
    }

    va_end(args);

    UNLOCK(self->mutex);
}


static inline void ArrayList_free(ArrayList *self) {
    LOCK(self->mutex);
    
    for (size_t i=0; i<self->capacity; ++i)
        Free(self->arr[i]);

    Free(self->arr);

    UNLOCK(self->mutex);

    pthread_mutex_destroy(&self->mutex);

    Free(self);
}

