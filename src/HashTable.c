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
static void hash_table_free(HashTable *self);


HashTable* New_HashTable() {
    HashTable *self = (HashTable*) Malloc(sizeof(HashTable));

    self->table = New_ArrayList();

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
    ArrayList *bucket = self->table->get_at(self->table, index);

    if (bucket) {
        for (size_t i=0; i<bucket->capacity; ++i) {
            Entry *entry = bucket->get_at(bucket, i);

            if (entry && strcmp(entry->key, key)==0) {
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
    ArrayList *bucket = self->table->get_at(self->table, index);

    if (!bucket) {
        bucket = New_ArrayList();
        self->table->set_at(self->table, bucket, sizeof(ArrayList), index);
    }

    for (size_t i=0; i<bucket->capacity; ++i) {
        Entry *entry = bucket->get_at(bucket, i);

        if (entry && strcmp(entry->key, key) == 0) {
            Free(entry->data);
            entry->data = copy_from_void_ptr(data, type_size);
            goto un;
        }
    }

    Entry *new_entry = (Entry*) malloc(sizeof(Entry));

    new_entry->key = strdup(key);
    new_entry->data = copy_from_void_ptr(data, type_size);

    for (size_t i=0; i<bucket->capacity; ++i) {
        if (!bucket->get_at(bucket, i)) {
            bucket->set_at(bucket, new_entry, sizeof(Entry), i);
            goto un;
        }
    }

    bucket->set_at(bucket, new_entry, sizeof(Entry), bucket->capacity);

    un:
        UNLOCK(self->mutex);
}


static inline void hash_table_delete(HashTable *self, char *key) {
    LOCK(self->mutex);

    size_t index = hash_index(key);
    ArrayList *bucket = self->table->get_at(self->table, index);

    if (!bucket)
        goto un;

    for (size_t i=0; i<bucket->capacity; ++i) {
        Entry *entry = bucket->get_at(bucket, i);

        if (entry && strcmp(entry->key, key)==0) {
            Free(entry->key);
            Free(entry->data);
            Free(entry);

            bucket->set_at(bucket, NULL, sizeof(void*), i);
            
            break;
        }
    }
    
    un:
        UNLOCK(self->mutex);
}


static void hash_table_free(HashTable *self) {
    LOCK(self->mutex);

    for (size_t i=0; i<self->table->capacity; ++i) {
        ArrayList *bucket = self->table->get_at(self->table, i);
        
        if (!bucket) 
            continue;

        for (size_t j=0; j<bucket->capacity; ++j) {
            Entry *entry = bucket->get_at(bucket, j);

            if (entry) {
                if (entry->key)
                    Free(entry->key);
                if (entry->data)
                    Free(entry->data);
            }
        }

        bucket->free(bucket);
    }

    self->table->free(self->table);

    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);

    Free(self);
}

