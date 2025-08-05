#include "HashTable.h"


static inline uint64_t fnv1a_64(const void *key, size_t len) {
    const uint8_t *data = (const uint8_t*) key;
    uint64_t hash = FNV_OFFSET_64;

    for (size_t i=0; i<len; ++i) {
        hash ^= data[i];
        hash *= FNV_PRIME_64;
    }

    return hash;
}


static inline size_t hash_index(const char *key) {
    return fnv1a_64(key, strlen(key)) % HASH_TABLE_CAPACITY;
}


HashTable* New_HashTable();
static inline void* hash_table_get(HashTable *self, char *key);
static inline void hash_table_set(HashTable *self, char *key, void *data, size_t type_size);
static inline void hash_table_delete(HashTable *self, char *key);
static void destroy_entry(void *data);
static void hash_table_free(HashTable *self);


HashTable* New_HashTable() {
    HashTable *self = (HashTable*) Malloc(sizeof(HashTable));

    self->table = New_ArrayList(NULL);

    pthread_mutexattr_init(&self->mutex_attr);
    pthread_mutexattr_settype(&self->mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&self->mutex, &self->mutex_attr);
    pthread_mutexattr_destroy(&self->mutex_attr);

    self->set = hash_table_set;
    self->get = hash_table_get;
    self->delete = hash_table_delete;
    self->free = hash_table_free;

    return self;
}


static inline void* hash_table_get(HashTable *self, char *key) {
    LOCK(self->mutex);

    void *res = NULL;
    size_t index = hash_index(key);

    ArrayList **bucket_ptr = self->table->get_at(self->table, index);
    ArrayList *bucket = bucket_ptr ? *bucket_ptr : NULL;

    if (bucket) {
        for (size_t i=0; i<bucket->capacity; ++i) {
            Entry **entry_ptr = bucket->get_at(bucket, i);
            Entry *entry = entry_ptr ? *entry_ptr : NULL;

            if (entry && strcmp(entry->key, key) == 0) {
                res = entry->data;
                break;
            }
        }
    }

    UNLOCK(self->mutex);

    return res;
}


static inline void hash_table_set(HashTable *self, char *key, void *data, size_t type_size) {
    LOCK(self->mutex);

    size_t index = hash_index(key);

    ArrayList **bucket_ptr = self->table->get_at(self->table, index);
    ArrayList *bucket = bucket_ptr ? *bucket_ptr : NULL;

    if (!bucket) {
        bucket = New_ArrayList(destroy_entry);
        self->table->set_at(self->table, &bucket, sizeof(void*), index);
    }

    ssize_t free_slot = -1;

    for (size_t i=0; i<bucket->capacity; ++i) {
        Entry **entry_ptr = bucket->get_at(bucket, i);
        Entry *entry = entry_ptr ? *entry_ptr : NULL;

        if (entry && strcmp(entry->key, key) == 0) {
            Free(entry->data);
            entry->data = copy_from_void_ptr(data, type_size);
            goto un;
        }

        if (!entry && free_slot == -1)
            free_slot = i;
    }

    Entry *new_entry = (Entry *)Malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->data = copy_from_void_ptr(data, type_size);

    if (free_slot == -1)
        free_slot = bucket->capacity;

    bucket->set_at(bucket, &new_entry, sizeof(void*), free_slot);

    un:
        UNLOCK(self->mutex);
}



static void destroy_entry(void *data) {
    Entry **entry_ptr = (Entry**) data;

    if (!entry_ptr || !(*entry_ptr)) 
        return;

    Entry *entry = (Entry*) (*entry_ptr);

    if (entry->key) 
        Free(entry->key);
    
    if (entry->data) 
        Free(entry->data);

    Free(entry);
    Free(entry_ptr);
}


static inline void hash_table_delete(HashTable *self, char *key) {
    LOCK(self->mutex);

    size_t index = hash_index(key);

    ArrayList **bucket_ptr = self->table->get_at(self->table, index);
    ArrayList *bucket = bucket_ptr ? *bucket_ptr : NULL;

    if (!bucket)
        goto un;

    for (size_t i=0; i<bucket->capacity; ++i) {
        Entry **entry_ptr = bucket->get_at(bucket, i);
        Entry *entry = entry_ptr ? *entry_ptr : NULL;

        if (entry && strcmp(entry->key, key) == 0) {
            bucket->set_at(bucket, NULL, sizeof(void*), i);
            break;
        }
    }

    un:
        UNLOCK(self->mutex);
}


static void hash_table_free(HashTable *self) {
    LOCK(self->mutex);

    for (size_t i = 0; i < self->table->capacity; ++i) {
        ArrayList **bucket_ptr = self->table->get_at(self->table, i);
        ArrayList *bucket = bucket_ptr ? *bucket_ptr : NULL;

        if (bucket)
            bucket->free(bucket);
    }

    self->table->free(self->table);

    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);

    Free(self);
}



