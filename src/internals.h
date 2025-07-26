#pragma once


#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdarg.h>
#include <errno.h>


#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)


#define LOCK(m) pthread_mutex_lock(&m)
#define UNLOCK(m) pthread_mutex_unlock(&m)


static inline void throw_memory_allocation_error(const char *file, int line, const char *function);
void *MallocWrapper(size_t size, const char *file, int line, const char *function);
void *CallocWrapper(size_t num, size_t size, const char *file, int line, const char *function);
void *ReallocWrapper(void *ptr, size_t size, const char *file, int line, const char *function);
void FreeWrapper(void *ptr, const char *file, int line, const char *function);



static inline void throw_memory_allocation_error(const char *file, int line, const char *function) {
    fprintf(stderr, "Error during memory allocation (%s) at %s:%d in function %s\n", strerror(errno), file, line, function);
    exit(EXIT_FAILURE);
}


void *MallocWrapper(size_t size, const char *file, int line, const char *function) {
    void *ptr = malloc(size);

    if (!ptr || size==0)
        throw_memory_allocation_error(file, line, function);

    return ptr;    
}


void *CallocWrapper(size_t num, size_t size, const char *file, int line, const char *function) {
    void *ptr = calloc(num, size);

    if (!ptr || size==0)
        throw_memory_allocation_error(file, line, function);

    return ptr;
}


void *ReallocWrapper(void *ptr, size_t size, const char *file, int line, const char *function) {
    void *new_ptr = realloc(ptr, size);

    if (!new_ptr || size==0)
        throw_memory_allocation_error(file, line, function);

    return new_ptr;
}


void FreeWrapper(void *ptr, const char *file, int line, const char *function) {
    #ifdef NULL_PTR_FREE_WARNING
    if (!ptr)
        fprintf(stderr, "Trying to free a null ptr at %s:%d in function %s\n", file, line, function);
    #endif

    free(ptr);    
}


#define MEM_DEBUG

#ifdef MEM_DEBUG

#define Malloc(n) MallocWrapper(n, __FILE__, __LINE__, __func__)
#define Calloc(n, s) CallocWrapper(n, s, __FILE__, __LINE__, __func__)
#define Realloc(p, n) ReallocWrapper(p, n, __FILE__, __LINE__, __func__)
#define Free(p) FreeWrapper(p, __FILE__, __LINE__, __func__)

#else
#define Malloc(n) Malloc(n)
#define Calloc(n, s) calloc(n)
#define Realloc(p, n) realloc(p, n)
#define Free(p) free(p)

#endif


extern char* strdup(const char*);


static inline void* copy_from_void_ptr(const void *src, size_t type_size) {
    if (!src || type_size == 0) 
        return NULL;

    void *copy = Malloc(type_size);

    memcpy(copy, src, type_size);
 
    return copy;
}


static inline bool compare_void_ptr(const void *ptr1, const void *ptr2, size_t type_size1, size_t type_size2) {
    return  (type_size1 == type_size2) && (memcmp(ptr1, ptr2, type_size1) == 0);
}


char* hex_string_from_pointer(void* data, size_t size) {
    if (!data || size == 0)
        return NULL;

    const unsigned char* bytes = (unsigned char*) data;

    char* hex_str = (char*) Malloc(size*2 + 1);
    
    for (size_t i = 0; i < size; ++i)
        sprintf(&hex_str[i * 2], "%02X", bytes[i]);

    hex_str[size * 2] = '\0';

    return hex_str;
}


