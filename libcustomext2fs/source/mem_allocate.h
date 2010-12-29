#ifndef _MEM_ALLOCATE_H
#define _MEM_ALLOCATE_H

#include <malloc.h>
#include "mem2.h"

extern __inline__ void* mem_alloc (size_t size) {
    return MEM2_alloc(size);
}

extern __inline__ void* mem_realloc (void *p, size_t size) {
    return MEM2_realloc(p, size);
}

extern __inline__ void* mem_align (size_t a, size_t size) {
    return MEM2_alloc(size);
}

extern __inline__ void mem_free (void* mem) {
    //using normal free, it will decide which free to use (just to be on the safe side)
    free(mem);
}

#endif /* _MEM_ALLOCATE_H */
