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


#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)


#define LOCK(m) pthread_mutex_lock(&m)
#define UNLOCK(m) pthread_mutex_unlock(&m)


extern char* strdup(const char*);


static inline void throw_memory_allocation_error() {
    fprintf(stderr, "Error during memory allocation\n");
    exit(EXIT_FAILURE);
}


static inline void* copy_from_void_ptr(const void *src, size_t type_size) {
    if (!src || type_size == 0) 
        return NULL;

    void *copy = malloc(type_size);
    
    if (!copy)
        throw_memory_allocation_error();

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

    char* hex_str = (char*) malloc(size*2 + 1);
    
    if (!hex_str)
        throw_memory_allocation_error();
    
    for (size_t i = 0; i < size; ++i)
        sprintf(&hex_str[i * 2], "%02X", bytes[i]);

    hex_str[size * 2] = '\0';

    return hex_str;
}


