#pragma once


#include "internals.h"
#include "HashTable.h"


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

