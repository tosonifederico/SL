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


void throw_memory_allocation_error(const char *file, int line, const char *function);
void *MallocWrapper(size_t size, const char *file, int line, const char *function);
void *CallocWrapper(size_t num, size_t size, const char *file, int line, const char *function);
void *ReallocWrapper(void *ptr, size_t size, const char *file, int line, const char *function);
void FreeWrapper(void *ptr, const char *file, int line, const char *function);


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


void* copy_from_void_ptr(const void *src, size_t type_size);


bool compare_void_ptr(const void *ptr1, const void *ptr2, size_t type_size1, size_t type_size2);


char* hex_string_from_pointer(void* data, size_t size);


