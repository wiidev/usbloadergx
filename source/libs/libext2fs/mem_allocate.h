#ifndef _MEM_ALLOCATE_H
#define _MEM_ALLOCATE_H

#include <malloc.h>

static inline void* mem_alloc (size_t size) {
    return malloc(size);
}

static inline void* mem_realloc (void *p, size_t size) {
    return realloc(p, size);
}

static inline void* mem_align (size_t size) {
    #ifdef __wii__
    return memalign(32, size);
    #else
    return malloc(size);
    #endif
}

static inline void mem_free (void* mem) {
    free(mem);
}

#endif /* _MEM_ALLOCATE_H */
