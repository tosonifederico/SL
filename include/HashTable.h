#pragma once


#include "internals.h"
#include "ArrayList.h"


// number of initial buckets
#define HASH_TABLE_CAPACITY 128


// hash contants
#define FNV_OFFSET_64 14695981039346656037ULL
#define FNV_PRIME_64  1099511628211ULL


typedef struct {
    char *key;
    void *data;
    size_t type_size;
} Entry;


typedef struct HashTable {
    struct HashTable *self;

    // ArrayList of double pointers to buckets
    // to avoid memory ownership error 
    // over the entry during the hashtable free
    // Likewise every buckets is an Arraylist
    // of double pointers to entry
    ArrayList *table;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    // returns the entry's data by its key
    void* (*get)(struct HashTable *self, char *key);

    // set/modify an entry's data by its key
    void (*set)(struct HashTable *self, char *key, void *data, size_t type_size);

    // delete an entry by its key
    void (*delete)(struct HashTable *self, char *key);

    // free the HashTable
    void (*free)(struct HashTable *self);

    // custom destroyer applied to every entry's data 
    void (*destroy)(void *data);
} HashTable;


HashTable* New_HashTable(void (*destroy_func)(void *data));



