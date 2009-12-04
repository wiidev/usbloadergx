// 2 MEM2 allocators, one for general purpose, one for covers
// Aligned and padded to 32 bytes, as required by many functions

#ifndef __MEM2_HPP
#define __MEM2_HPP

#ifdef __cplusplus
extern "C"
{
#endif

void *MEM2_alloc(unsigned int s);
void *MEM2_realloc(void *p, unsigned int s);
void MEM2_free(void *p);
unsigned int MEM2_usableSize(void *p);

#ifdef __cplusplus
}

void MEM2_init(unsigned int mem2Size);
void MEM2_cleanup(void);
void MEM2_takeBigOnes(bool b);

#endif

#endif // !defined(__MEM2_HPP)
