#ifndef _MEM_ALLOCATE_H
#define _MEM_ALLOCATE_H

#include <malloc.h>
#include "mem2.h"

extern __inline__ void* mem_alloc (size_t size) {
    return MEM2_alloc(size);
}

extern __inline__ void* mem_calloc (size_t count, size_t size) {
	void *p = MEM2_alloc(count * size);
	if(p) {
		memset(p, 0, count * size);
	}
    return p;
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
