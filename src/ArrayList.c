#include "ArrayList.h"


ArrayList* New_ArrayList(void (*destroy_func)(void *data));
static inline void ArrayList_set_at(ArrayList *self, void *data, size_t type_size, size_t index);
static inline void* ArrayList_get_at(ArrayList *self, size_t index);
static void ArrayList_foreach(ArrayList *self, void (*func)(void *data, va_list args), ...);
static inline void ArrayList_free(ArrayList *self);


ArrayList* New_ArrayList(void (*destroy_func)(void *data)) {
    ArrayList *self = (ArrayList*) Malloc(sizeof(ArrayList));

    // uses recursive mutex to ensure 
    // that we can call functions from other function
    // blocking multiple times the same mutex
    // without crashing
    pthread_mutexattr_init(&self->mutex_attr);
    pthread_mutexattr_settype(&self->mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&self->mutex, &self->mutex_attr);

    pthread_mutexattr_destroy(&self->mutex_attr);

    self->capacity = ARRAY_LIST_INITIAL_CAPACITY;
    self->arr = (ArrayListElement**) Calloc(self->capacity, sizeof(ArrayListElement*));
    
    self->set_at = ArrayList_set_at;
    self->get_at = ArrayList_get_at;
    self->foreach = ArrayList_foreach;
    self->free = ArrayList_free;

    // if a destroyer function is not provided we use (free)
    self->destroy = destroy_func ? destroy_func : free;

    return self;
}


static inline void ArrayList_set_at(ArrayList *self, void *data, size_t type_size, size_t index) {
    LOCK(self->mutex);

    // if the index is out of bounds
    // double the arraylist capacity
    // unitl index < self->capacity
    if (index >= self->capacity) {
        size_t new_capacity = self->capacity * 2;
        while (index >= new_capacity)
            new_capacity *= 2;

        ArrayListElement **new_arr = (ArrayListElement**) Realloc(self->arr, new_capacity * sizeof(ArrayListElement*));

        memset(new_arr+self->capacity, 0, (new_capacity-self->capacity) * sizeof(ArrayListElement*));

        self->arr = new_arr;
        self->capacity = new_capacity;
    }

    // if at index there's no ArrayListElement
    // allocates it
    // otherwise destroy the data
    if (!self->arr[index]) {
        self->arr[index] = (ArrayListElement*) Malloc(sizeof(ArrayListElement));

        self->arr[index]->data = NULL;
        self->arr[index]->type_size = 0;
    } else {
        if (self->arr[index]->data)
            self->destroy(self->arr[index]->data);
    }

    // update ArrayListElement at index
    self->arr[index]->data = copy_from_void_ptr(data, type_size);
    self->arr[index]->type_size = type_size;

    UNLOCK(self->mutex);
}



static inline void* ArrayList_get_at(ArrayList *self, size_t index) {
    LOCK(self->mutex);

    void *val = NULL;

    if (index < self->capacity && self->arr[index])
        val = copy_from_void_ptr(self->arr[index]->data, self->arr[index]->type_size);

    UNLOCK(self->mutex);

    return val;
}


static void ArrayList_foreach(ArrayList *self, void (*func)(void *data, va_list args), ...) {
    LOCK(self->mutex);

    va_list args;
    va_start(args, func);

    for (size_t i=0; i<self->capacity; ++i) {
        if (!self->arr[i])
            continue;

        va_list args_copy;
        va_copy(args_copy, args);
        func(self->arr[i]->data, args_copy);
        va_end(args_copy);
    }

    va_end(args);

    UNLOCK(self->mutex);
}


static inline void ArrayList_free(ArrayList *self) {
    LOCK(self->mutex);

    for (size_t i=0; i<self->capacity; ++i) {
        // check if at i there's an ArrayListElement
        if (self->arr[i]) {
            // check if the element as data
            // ad if so destroies it
            if (self->arr[i]->data)
                self->destroy(self->arr[i]->data);

            // Free the ArrayListElement
            Free(self->arr[i]);
        }
    }

    // Free the arr
    Free(self->arr);

    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);

    // Free the ArrayList
    free(self);
}


