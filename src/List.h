#pragma once


#include "./internals.h"


typedef struct list_node {
    void *data;
    size_t type_size;

    struct list_node *next;
    struct list_node *prev;
} list_node;


typedef struct List {
    struct List *self;

    list_node *head;
    list_node *tail;
    size_t len;

    pthread_mutex_t mutex;
    pthread_mutexattr_t mutex_attr;

    bool (*is_empty)(struct List *self);
    void (*print)(struct List *self);
    void (*append_at)(struct List *self, void *data, size_t type_size, size_t index);
    void (*modify_at)(struct List *self, void *data, size_t type_size, size_t index);
    void* (*get_at)(struct List *self, size_t index);
    void (*reverse)(struct List *self);
    void (*foreach)(struct List *self, void (*func)(void *data, va_list args), ...);
    void* (*delete_at)(struct List *self, size_t index);
    void (*push)(struct List *self, void *data, size_t type_size);
    void* (*pop)(struct List *self);
    void (*enqueue)(struct List *self, void *data, size_t type_size);
    void* (*dequeue)(struct List *self);
    bool (*lookup)(struct List *self, void *data, size_t type_size);
    size_t (*count_occurrences)(struct List *self, void *data, size_t type_size);
    void (*merge)(struct List *self, struct List *l2);
    void (*free)(struct List *self);
} List;


List* New_List();
static list_node* init_list_node(void *data, size_t type_size);
static inline bool list_is_empty(List *self);
static inline void list_print(List *self);
static inline void list_append_at(List *self, void *data, size_t type_size, size_t index);
static inline void list_modify_at(List *self, void *data, size_t type_size, size_t index);
static inline void* list_get_at(List *self, size_t index);
static inline void list_reverse(List *self);
static inline void list_foreach(List *self, void (*func)(void *data, va_list args), ...);
static inline void* list_delete_at(List *self, size_t index);
static inline void list_push(List *self, void *data, size_t type_size);
static inline void* list_pop(List *self);
static inline void list_enqueue(List *self, void *data, size_t type_size);
static inline void* list_dequeue(List *self);
static inline bool list_lookup(List *self, void *data, size_t type_size);
static inline size_t list_count_occurrences(struct List *self, void *data, size_t type_size);
static inline void list_merge(List *self, List *l2);
static inline void list_free(List *self);


List* New_List() {
    List *self = (List*) Malloc(sizeof(List));

    self->self = self;

    self->len = 0;
    self->head = NULL;
    self->tail = NULL;
    pthread_mutexattr_init(&self->mutex_attr);
    pthread_mutexattr_settype(&self->mutex_attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&self->mutex, &self->mutex_attr);

    pthread_mutexattr_destroy(&self->mutex_attr);

    self->is_empty = list_is_empty;
    self->print = list_print;
    self->append_at = list_append_at;
    self->modify_at = list_modify_at;
    self->get_at = list_get_at;
    self->reverse = list_reverse;
    self->foreach = list_foreach;
    self->delete_at = list_delete_at;
    self->push = list_push;
    self->pop = list_pop;
    self->enqueue = list_enqueue;
    self->dequeue = list_dequeue;
    self->lookup = list_lookup;
    self->count_occurrences = list_count_occurrences;
    self->merge = list_merge;
    self->free = list_free;

    return self;
}


static list_node* init_list_node(void *data, size_t type_size) {
    list_node *node = (list_node*) Malloc(sizeof(list_node));

    node->data = data;
    node->type_size = type_size;
    node->next = NULL;
    node->prev = NULL;
    
    return node;
}


static inline bool list_is_empty(List *self) {
    LOCK(self->mutex);

    size_t len = self->len;
    
    UNLOCK(self->mutex);
    return len == 0;
}


static inline void list_print(List *self) {
    LOCK(self->mutex);

    if (self->is_empty(self)) {
        printf("Empty list\n");
        goto un;
    }

    list_node *current_node = self->head;
    
    for (size_t i=0; i<self->len; ++i) {
        printf("|%d| -> ", *((int*)current_node->data));
        current_node = current_node->next;
    }

    printf("NULL\n");

    un:
        UNLOCK(self->mutex);
}


static inline void list_append_at(List *self, void *data, size_t type_size, size_t index) {
    LOCK(self->mutex);

    if (index > self->len) {
        fprintf(stderr, "Appending List element out of bound\n");
        exit(EXIT_FAILURE);
    }

    list_node *new_node = init_list_node(copy_from_void_ptr(data, type_size), type_size);

    if (index == 0) {
        new_node->prev = NULL;
        new_node->next = self->head;

        if (self->head)
            self->head->prev = new_node;
        else
            self->tail = new_node;

        self->head = new_node;
    } else if (index == self->len) {
        new_node->next = NULL;
        new_node->prev = self->tail;

        self->tail->next = new_node;
        self->tail = new_node;
    } else {
        list_node *prev_node;

        if (index < self->len/2) {
            prev_node = self->head;

            for (size_t i=0; i<index-1; ++i)
                prev_node = prev_node->next;
        } else {
            prev_node = self->tail;

            for (size_t i=self->len-1; i>index-1; --i)
                prev_node = prev_node->prev;
        }

        new_node->prev = prev_node;
        new_node->next = prev_node->next;

        prev_node->next = new_node;
        new_node->next->prev = new_node;
    }

    self->len++;

    UNLOCK(self->mutex);
}


static inline void list_modify_at(List *self, void *data, size_t type_size, size_t index) {
    LOCK(self->mutex);

    if (index >= self->len) {
        fprintf(stderr, "Setting List element out of bound\n");
        exit(EXIT_FAILURE);
    }

    list_node *current_node;

    if (index == self->len-1) 
        current_node = self->tail;
    else {
        if (index < self->len/2) {
            current_node = self->head;

            for (size_t i=0; i<index; ++i)
                current_node = current_node->next;
        } else {
            current_node = self->tail;

            for (size_t i=self->len-1; i>index; --i)
                current_node = current_node->prev;
        }
    }

    Free(current_node->data);
    current_node->data = copy_from_void_ptr(data, type_size);

    UNLOCK(self->mutex);
}


static inline void* list_get_at(List *self, size_t index) {
    LOCK(self->mutex);

    if (index >= self->len) {
        fprintf(stderr, "Indexing List out of bound\n");
        exit(EXIT_FAILURE);
    }

    list_node *current_node = self->head;
    
    if (index == self->len-1)
        current_node = self->tail;
    else {
        if (index < self->len/2) {
            current_node = self->head;

            for (size_t i=0; i<index; ++i)
                current_node = current_node->next;
        } else {
            current_node = self->tail;

            for (size_t i=self->len-1; i>index; --i)
                current_node = current_node->prev;
        }
    }

    void *data = current_node->data;
    
    UNLOCK(self->mutex);

    return data;
}


static inline void list_reverse(List *self) {
    LOCK(self->mutex);

    list_node *current_node = self->head;
    list_node *temp = NULL;

    while (current_node != NULL) {
        temp = current_node->prev;
        
        current_node->prev = current_node->next;
        current_node->next = temp;
        current_node = current_node->prev;
    }

    if (temp != NULL) {
        self->tail = self->head;
        self->head = temp->prev;
    }

    UNLOCK(self->mutex);
}


static inline void list_foreach(List *self, void (*func)(void *data, va_list args), ...) {
    LOCK(self->mutex);

    va_list args;
    va_start(args, func);

    list_node *current_node = self->head;

    for (size_t i=0; i<self->len; ++i) {
        va_list args_copy;
        va_copy(args_copy, args);
        func(current_node->data, args_copy);
        va_end(args_copy);

        current_node = current_node->next;
    }

    va_end(args);

    UNLOCK(self->mutex);
}


static inline void* list_delete_at(List *self, size_t index) {
    LOCK(self->mutex);

    if (index >= self->len) {
        fprintf(stderr, "Deleting List index out of bound\n");
        exit(EXIT_FAILURE);
    }

    list_node *to_delete = NULL;

    if (index == 0) {
        to_delete = self->head;
        self->head = self->head->next;
        
        if (self->head)
            self->head->prev = NULL;
        else
            self->tail = self->head;    
    } else if (index == self->len-1) {
        to_delete = self->tail;
        self->tail = self->tail->prev;

        if (self->tail)
            self->tail->next = NULL;
        else
            self->head = NULL;
    } else {
        list_node *prev_node;
        list_node *next_node;

        if (index < self->len/2) {
            prev_node = self->head;

            for (size_t i=0; i<index-1; ++i)
                prev_node = prev_node->next;
        } else {
            prev_node = self->tail;

            for (size_t i=self->len-1; i>index-1; --i)
                prev_node = prev_node->prev;
        }

        to_delete = prev_node->next;

        next_node = to_delete->next;

        prev_node->next = to_delete->next;
        next_node->prev = prev_node;
    }

    self->len--;

    void *ret = copy_from_void_ptr(to_delete->data, to_delete->type_size);
    
    Free(to_delete->data);
    Free(to_delete);

    UNLOCK(self->mutex);

    return ret;
}


static inline void list_push(List *self, void *data, size_t type_size) {
    self->append_at(self, data, type_size, self->len);
}


static inline void* list_pop(List *self) {
    if (self->is_empty(self))
        return NULL;
    return self->delete_at(self, (self->len!=0) ? self->len-1 : 0);
}


static inline void list_enqueue(List *self, void *data, size_t type_size) {
    self->append_at(self, data, type_size, 0);
}


static inline void* list_dequeue(List *self) {
    if (self->is_empty(self))
        return NULL;
    return self->delete_at(self, (self->len!=0) ? self->len-1 : 0);
}


static inline bool list_lookup(List *self, void *data, size_t type_size) {
    LOCK(self->mutex);

    list_node *current_node = self->head;
    bool found = false;

    for (size_t i=0; i<self->len && !found; ++i) {
        if (compare_void_ptr(current_node->data, data, current_node->type_size, type_size))
            found = true;

        current_node = current_node->next;
    }

    UNLOCK(self->mutex);

    return found;
}


static inline size_t list_count_occurrences(struct List *self, void *data, size_t type_size) {
    LOCK(self->mutex);

    size_t count = 0;

    list_node *current_node = self->head;

    for (size_t i=0; i<self->len; ++i) {
        if (compare_void_ptr(current_node->data, data, current_node->type_size, type_size))
            count++;

        current_node = current_node->next;
    }

    UNLOCK(self->mutex);

    return count;
}


static inline void list_merge(List *self, List *l2) {
    list_node *current_node = l2->head;

    while (current_node) {
        self->push(self, current_node->data, current_node->type_size);
        current_node = current_node->next;  
    }
}


static inline void list_free(List *self) {
    LOCK(self->mutex);

    while (!self->is_empty(self))
        self->delete_at(self, 0);

    UNLOCK(self->mutex);
    pthread_mutex_destroy(&self->mutex);

    Free(self);
}


