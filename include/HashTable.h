#pragma once


#include "internals.h"
#include "ArrayList.h"


#define HASH_TABLE_CAPACITY 128


#define FNV_OFFSET_64 14695981039346656037ULL
#define FNV_PRIME_64  1099511628211ULL


typedef struct entry {
    char *key;
    void *data;
} Entry;


typedef struct HashTable {
    struct HashTable *self;

    ArrayList *table;
    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    void* (*get)(struct HashTable *self, char *key);
    void (*set)(struct HashTable *self, char *key, void *data, size_t type_size);
    void (*delete)(struct HashTable *self, char *key);
    void (*free)(struct HashTable *self);
} HashTable;


HashTable* New_HashTable();



