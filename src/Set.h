#pragma once


#include "./internals.h"
#include "./HashTable.h"


typedef struct Set {
    struct Set *self;

    HashTable *table;

    char *flag;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    void (*insert)(struct Set *self, void *data, size_t type_size);
    void (*delete)(struct Set *self, void *data, size_t type_size);
    bool (*lookup)(struct Set *self, void *data, size_t type_size);
    void (*merge)(struct Set *self, struct Set *s2);
    void (*free)(struct Set *self);    
} Set;


Set* New_Set(); 
static inline void set_insert(Set *self, void *data, size_t type_size);
static inline void set_delete(Set *self, void *data, size_t type_size);
static inline bool set_lookup(Set *self, void *data, size_t type_size);
static inline void set_merge(Set *self, Set *s2);
static void set_free( Set *self);


Set* New_Set() {
    Set *self = (Set*) malloc(sizeof(Set));

    if (!self)
        throw_memory_allocation_error();

    self->self = self;
    
    self->table = New_HashTable();

    self->flag = (char*) malloc(sizeof(char));
    *(self->flag) = 'A';

    pthread_mutexattr_init(&self->mutex_attr);
    pthread_mutexattr_settype(&self->mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&self->mutex, &self->mutex_attr);

    pthread_mutexattr_destroy(&self->mutex_attr);

    self->insert = set_insert;
    self->delete = set_delete;
    self->lookup = set_lookup;
    self->merge = set_merge;
    self->free = set_free;

    return self;
}


static inline void set_insert(Set *self, void *data, size_t type_size) {
    LOCK(self->mutex);

    char *key = hex_string_from_pointer(data, type_size);
    char *entry_data = (char*) self->table->get(self->table, key);

    if (entry_data && *entry_data == *(self->flag))
        return;
    else
        self->table->set(self->table, key, self->flag, type_size);
    
    UNLOCK(self->mutex);
}


static inline void set_delete(Set *self, void *data, size_t type_size) {
    LOCK(self->mutex);
    
    self->table->delete(self->table, hex_string_from_pointer(data, type_size));
    
    UNLOCK(self->mutex);
}


static inline bool set_lookup(Set *self, void *data, size_t type_size) {
    LOCK(self->mutex);

    char *entry_data = (char*) self->table->get(self->table, hex_string_from_pointer(data, type_size));
    bool res = entry_data && *entry_data == *(self->flag);
    
    UNLOCK(self->mutex);

    return res;
}


static inline void set_merge(Set *self, Set *s2) {
    LOCK(self->mutex);
    LOCK(s2->mutex);

    

    UNLOCK(self->mutex);
    UNLOCK(s2->mutex);
}


static void set_free(Set *self) {
    LOCK(self->mutex);

    free(self->flag);
    self->table->free(self->table);
    
    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);

    free(self);
}

