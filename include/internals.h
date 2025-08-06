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


// macros to lock and unlock any mutex
#define LOCK(m) pthread_mutex_lock(&m)
#define UNLOCK(m) pthread_mutex_unlock(&m)


// prints an error to stderr when a memory allocation fails 
void throw_memory_allocation_error(const char *file, int line, const char *function);


// Wappers to memory allocation function to show in case of failure
// file, line, and function in which the problem occurred
void *MallocWrapper(size_t size, const char *file, int line, const char *function);
void *CallocWrapper(size_t num, size_t size, const char *file, int line, const char *function);
void *ReallocWrapper(void *ptr, size_t size, const char *file, int line, const char *function);
void FreeWrapper(void *ptr, const char *file, int line, const char *function);


// macros to link memory function to their respective wrappers
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


/*
    given a void* returns a copy of the ptr allocated in the heap.
    It doesn't perfom deep copies.
    if src is NULL or type_size is 0 returns NULL
*/
void* copy_from_void_ptr(const void *src, size_t type_size);


// returns true if two void* points to two equal chunks of memory
bool compare_void_ptr(const void *ptr1, const void *ptr2, size_t type_size1, size_t type_size2);


// given a void*, read the first (size) bytes and returns the hex representation in a null-terminated string
char* hex_string_from_pointer(void* data, size_t size);


