#pragma once


#include "./internals.h"
#include "./AVL_Tree.h"


char* pointed_to_hex_string(void* data, size_t size) {
    if (!data || size == 0)
        return NULL;

    const unsigned char* bytes = (unsigned char*) data;

    char* hex_str = (char*) malloc(size*2 + 1);
    
    if (!hex_str)
        throw_memory_allocation_error();
    
    for (size_t i = 0; i < size; ++i)
        sprintf(&hex_str[i * 2], "%02X", bytes[i]);

    hex_str[size * 2] = '\0';

    return hex_str;
}


int get_key_from_data(void *data, size_t size) {
    char *hex_string = pointed_to_hex_string(data, size);
    int key = fnv1a_64(hex_string, strlen(hex_string));

    free(hex_string);

    return key;
}


typedef struct Set {
    struct Set *self;

    AVL_Tree *tree;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    void (*insert)(struct Set *self, void *data, size_t type_size);
    void (*delete)(struct Set *self, void *data, size_t type_size);
    bool (*lookup)(struct Set *self, void *data, size_t type_size);
    void (*free)(struct Set *self);    
} Set;


Set* New_Set(); 
static inline void set_insert(Set *self, void *data, size_t type_size);
static inline void set_delete(Set *self, void *data, size_t type_size);
static inline bool set_lookup(Set *self, void *data, size_t type_size);
static void set_free( Set *self);    


Set* New_Set() {
    Set *self = (Set*) malloc(sizeof(Set));

    if (!self)
        throw_memory_allocation_error();

    self->self = self;
    
    self->tree = New_AVL_Tree();

    pthread_mutexattr_init(&self->mutex_attr);
    pthread_mutexattr_settype(&self->mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&self->mutex, &self->mutex_attr);

    pthread_mutexattr_destroy(&self->mutex_attr);

    self->insert = set_insert;
    self->delete = set_delete;
    self->lookup = set_lookup;
    self->free = set_free;

    return self;
}


static inline void set_insert(Set *self, void *data, size_t type_size) {
    LOCK(self->mutex);

    self->tree->insert(self->tree, get_key_from_data(data, type_size), data, type_size);
    
    UNLOCK(self->mutex);
}


static inline void set_delete(Set *self, void *data, size_t type_size) {
    LOCK(self->mutex);
    
    self->tree->delete(self->tree, get_key_from_data(data, type_size));
    
    UNLOCK(self->mutex);
}


static inline bool set_lookup(Set *self, void *data, size_t type_size) {
    LOCK(self->mutex);

    bool res =  self->tree->lookup(self->tree, get_key_from_data(data, type_size)) != NULL;
    
    UNLOCK(self->mutex);

    return res;
}


static void set_free(Set *self) {
    LOCK(self->mutex);

    self->tree->free(self->tree);
    
    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);

    free(self);
}

