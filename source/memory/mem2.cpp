
#include "mem2.h"
#include "mem2alloc.h"
#include "gecko.h"

#include <malloc.h>
#include <string.h>

#define MEM2_PRIORITY_SIZE	0x40

// Forbid the use of MEM2 through malloc
u32 MALLOC_MEM2 = 0;

static CMEM2Alloc g_mem2gp;

void MEM2_init(unsigned int mem2Size)
{
	g_mem2gp.init(mem2Size);
}

void MEM2_cleanup(void)
{
	g_mem2gp.cleanup();
}

extern "C" void *MEM2_alloc(unsigned int s)
{
	return g_mem2gp.allocate(s);
}

extern "C" void MEM2_free(void *p)
{
	g_mem2gp.release(p);
}

extern "C" void *MEM2_realloc(void *p, unsigned int s)
{
	return g_mem2gp.reallocate(p, s);
}

extern "C" unsigned int MEM2_usableSize(void *p)
{
	return CMEM2Alloc::usableSize(p);
}

// Give priority to MEM2 for big allocations
// Used for saving some space in malloc, which is required for 2 reasons :
// - decent speed on small and frequent allocations
// - newlib uses its malloc internally (for *printf for example) so it should always have some memory left
bool g_bigGoesToMem2 = false;

void MEM2_takeBigOnes(bool b)
{
	g_bigGoesToMem2 = b;
}


extern "C"
{

extern __typeof(malloc) __real_malloc;
extern __typeof(calloc) __real_calloc;
extern __typeof(realloc) __real_realloc;
extern __typeof(memalign) __real_memalign;
extern __typeof(free) __real_free;
extern __typeof(malloc_usable_size) __real_malloc_usable_size;

void *__wrap_malloc(size_t size)
{
	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		p = MEM2_alloc(size);
		if (p != 0) {
			gprintf("Malloc of size %d returns address in MEM2\n", size);
			return p;
		}
		gprintf("Malloc of size %d returns address in MEM1\n", size);
		return __real_malloc(size);
	}
	p = __real_malloc(size);
	if (p != 0) {
		gprintf("Malloc of size %d returns address in MEM1\n", size);
		return p;
	}
	gprintf("Malloc of size %d returns address in MEM2\n", size);
	return MEM2_alloc(size);
}

void *__wrap_calloc(size_t n, size_t size)
{
	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		p = MEM2_alloc(n * size);
		if (p != 0)
		{
			gprintf("Calloc of amount %d, size %d returns address in MEM2\n", n, size);
			memset(p, 0, n * size);
			return p;
		}
		gprintf("Calloc of amount %d, size %d returns address in MEM1\n", n, size);
		return __real_calloc(n, size);
	}
	p = __real_calloc(n, size);
	if (p != 0) {
		gprintf("Calloc of amount %d, size %d returns address in MEM1\n", n, size);
		return p;
	}
	p = MEM2_alloc(n * size);
	if (p != 0) {
		gprintf("Calloc of amount %d, size %d returns address in MEM2\n", n, size);
		memset(p, 0, n * size);
	} else {
		gprintf("Calloc of amount %d, size %d returns NULL\n", n, size);
	}
	return p;
}

void *__wrap_memalign(size_t a, size_t size)
{
	void *p;
	if (g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE)
	{
		if (a <= 32 && 32 % a == 0)
		{
			p = MEM2_alloc(size);
			if (p != 0) {
				gprintf("Memalign in blocks of %d, size %d returns address in MEM2\n", a, size);
				return p;
			}
		}
		gprintf("Memalign in blocks of %d, size %d returns address in MEM1\n", a, size);
		return __real_memalign(a, size);
	}
	p = __real_memalign(a, size);
	if (p != 0 || a > 32 || 32 % a != 0) {
		gprintf("Memalign in blocks of %d, size %d returns address in MEM1\n", a, size);
		return p;
	}

	p = MEM2_alloc(size);
	if (p != 0) {
		gprintf("Memalign in blocks of %d, size %d returns address in MEM2\n", a, size);
	} else {
		gprintf("Memalign in blocks of %d, size %d returns NULL\n", a, size);
	}
	return p;
}

void __wrap_free(void *p)
{
	if (((u32)p & 0x10000000) != 0) {
		gprintf("Free pointer in address in MEM2\n");
		MEM2_free(p);
	} else {
		gprintf("Free pointer in address in MEM1\n");
		__real_free(p);
	}
}

void *__wrap_realloc(void *p, size_t size)
{
	void *n;
	// ptr from mem2
	if (((u32)p & 0x10000000) != 0 || (p == 0 && g_bigGoesToMem2 && size > MEM2_PRIORITY_SIZE))
	{
		n = MEM2_realloc(p, size);
		if (n != 0) {
			gprintf("Realloc of size %d returns memory in MEM2\n", size);
			return n;
		}
		n = __real_malloc(size);
		if (n == 0) {
			gprintf("Realloc of size %d returns NULL\n", size);
			return 0;
		}
		if (p != 0)
		{
			memcpy(n, p, MEM2_usableSize(p) < size ? MEM2_usableSize(p) : size);
			MEM2_free(p);
		}
		gprintf("Realloc of size %d returns memory in MEM1\n", size);
		return n;
	}
	// ptr from malloc
	n = __real_realloc(p, size);
	if (n != 0) {
		gprintf("Realloc of size %d returns memory in MEM1\n", size);
		return n;
	}
	n = MEM2_alloc(size);
	if (n == 0) {
		gprintf("Realloc of size %d returns memory in MEM2\n", size);
		return 0;
	}
	if (p != 0)
	{
		memcpy(n, p, __real_malloc_usable_size(p) < size ? __real_malloc_usable_size(p) : size);
		__real_free(p);
	}
	gprintf("Realloc of size %d returns memory in MEM2\n", size);
	return n;
}

size_t __wrap_malloc_usable_size(void *p)
{
	if (((u32)p & 0x10000000) != 0)
		return MEM2_usableSize(p);
	return __real_malloc_usable_size(p);
}

}
